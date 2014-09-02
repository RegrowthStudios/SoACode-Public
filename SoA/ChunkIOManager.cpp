#include "stdafx.h"

#include "ChunkIOManager.h"

#include <direct.h> //for mkdir windows
#include <dirent.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include <thread>

#include <ZLIB/zlib.h>

#include "BlockData.h"
#include "Chunk.h"
#include "Errors.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "Options.h"
#include "Planet.h"
#include "Player.h"

ChunkIOManager::ChunkIOManager()
{
    _isThreadFinished = 0;
    readWriteThread = NULL;
    _shouldDisableLoading = 0;
}

ChunkIOManager::~ChunkIOManager()
{
    onQuit();
}

void ChunkIOManager::clear() {
    
    _queueLock.lock();
    queue<Chunk*>().swap(chunksToLoad); //clear the queue
    _queueLock.unlock();

    while (GameManager::chunkIOManager->getSaveListSize() != 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    flcLock.lock();
    vector<Chunk*>().swap(finishedLoadChunks);
    flcLock.unlock();
}


void ChunkIOManager::addToSaveList(Chunk *ch)
{
    if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
        ch->dirty = 0;
        _queueLock.lock();
        ch->inSaveThread = 1;
        chunksToSave.push(ch);
        _queueLock.unlock();
        _cond.notify_one();
    }
}

void ChunkIOManager::addToSaveList(vector <Chunk *> &chunks)
{
    _queueLock.lock();
    Chunk *ch;
    for (size_t i = 0; i < chunks.size(); i++){
        ch = chunks[i];
        if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
            ch->inSaveThread = 1;
            ch->dirty = 0;
            chunksToSave.push(ch);
        }
    }
    _queueLock.unlock();
    _cond.notify_one();
}

void ChunkIOManager::addToLoadList(Chunk *ch)
{
    if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
        _queueLock.lock();
        ch->loadStatus = 0;
        ch->inLoadThread = 1;
        chunksToLoad.push(ch);
        _queueLock.unlock();
        _cond.notify_one();
    }
}

void ChunkIOManager::addToLoadList(vector <Chunk *> &chunks)
{
    _queueLock.lock();
    Chunk *ch;

    if (_shouldDisableLoading) {
        flcLock.lock();
        for (size_t i = 0; i < chunks.size(); i++){
            chunks[i]->loadStatus = 2;
            finishedLoadChunks.push_back(chunks[i]);
        }
        flcLock.unlock();
        _queueLock.unlock();
        return;
    }

    for (size_t i = 0; i < chunks.size(); i++){
        ch = chunks[i];
       
        if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
            ch->inLoadThread = 1;
            chunksToLoad.push(ch);
        }
        else{
            cout << "ERROR: Tried to add chunk to load list and its in a thread! : " << ch->position.x << " " << ch->position.y << " " << ch->position.z << endl;
        }
    }
    _queueLock.unlock();
    _cond.notify_one();
}

int ChunkIOManager::getLoadListSize()
{    
    return chunksToLoad.size();
}

int ChunkIOManager::getSaveListSize()
{
    int rv;
    _queueLock.lock();
    rv = chunksToSave.size();
    _queueLock.unlock();
    return rv;
}

void ChunkIOManager::readWriteChunks()
{
    unique_lock<mutex> queueLock(_queueLock);
    Chunk *ch;

    string reg;

    while (!_isDone){
        if (_isDone){
            _regionFileManager.clear();
            queueLock.unlock();
            _isThreadFinished = 1;
            return;
        }
        _regionFileManager.flush();
        _cond.wait(queueLock); //wait for a notification that queue is not empty

        if (_isDone){
            _regionFileManager.clear();
            queueLock.unlock();
            _isThreadFinished = 1;
            return;
        }
        while (chunksToLoad.size() || chunksToSave.size()){ //loops through the load and save queues
            if (chunksToLoad.size()){ //do load list first

                ch = chunksToLoad.front();
                chunksToLoad.pop();
                queueLock.unlock();

             //   if (_regionFileManager.tryLoadChunk(ch) == false) {
                    ch->loadStatus = 1;
             //   }

                flcLock.lock();
                finishedLoadChunks.push_back(ch);
                flcLock.unlock();

                queueLock.lock();
            }
            else if (chunksToSave.size()){
                ch = chunksToSave.front();
                queueLock.unlock();

          //      _regionFileManager.saveChunk(ch);
            
                queueLock.lock();
                chunksToSave.pop();
                ch->inSaveThread = 0; //race condition! make a new queue!
            }
        }
    }
}

void ChunkIOManager::beginThread()
{
    _isDone = 0;
    _isThreadFinished = 0;
    readWriteThread = NULL;
    readWriteThread = new std::thread(&ChunkIOManager::readWriteChunks, this);
}

void ChunkIOManager::onQuit()
{

    clear();

    _queueLock.lock();
    _isDone = 1;
    _queueLock.unlock();
    _cond.notify_one();
    if (readWriteThread != NULL && readWriteThread->joinable()) readWriteThread->join();
    delete readWriteThread;
    readWriteThread = NULL;
    
    finishedLoadChunks.clear();
}

bool ChunkIOManager::saveVersionFile() {
    return _regionFileManager.saveVersionFile();
}

bool ChunkIOManager::checkVersion() {
    return _regionFileManager.checkVersion();
}