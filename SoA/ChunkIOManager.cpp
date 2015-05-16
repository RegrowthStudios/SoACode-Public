#include "stdafx.h"

#include "ChunkIOManager.h"

#include <direct.h> //for mkdir windows
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
#include "SoaOptions.h"

ChunkIOManager::ChunkIOManager(const nString& saveDir) :
    _regionFileManager(saveDir)
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
    Chunk* tmp;
    _queueLock.lock();
    //flush queues
    while (chunksToLoad.try_dequeue(tmp));
    _queueLock.unlock();

    while (chunksToSave.peek() != nullptr);

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    while (finishedLoadChunks.try_dequeue(tmp));
}


void ChunkIOManager::addToSaveList(Chunk *ch)
{
    if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
        ch->dirty = 0;
        ch->inSaveThread = 1;
        chunksToSave.enqueue(ch);
        _cond.notify_one();
    }
}

void ChunkIOManager::addToSaveList(std::vector <Chunk *> &chunks)
{
    Chunk *ch;
    for (size_t i = 0; i < chunks.size(); i++){
        ch = chunks[i];
        if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
            ch->inSaveThread = 1;
            ch->dirty = 0;
            chunksToSave.enqueue(ch);
        }
    }
    _cond.notify_one();
}

void ChunkIOManager::addToLoadList(Chunk *ch)
{
    if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
        ch->loadStatus = 0;
        ch->inLoadThread = 1;
        chunksToLoad.enqueue(ch);
        _cond.notify_one();
    }
}

void ChunkIOManager::addToLoadList(std::vector <Chunk *> &chunks)
{
    Chunk *ch;

    if (_shouldDisableLoading) {
        for (size_t i = 0; i < chunks.size(); i++){
            chunks[i]->loadStatus = 2;
            finishedLoadChunks.enqueue(chunks[i]);
        }
        return;
    }

    for (size_t i = 0; i < chunks.size(); i++){
        ch = chunks[i];
       
        if (ch->inSaveThread == 0 && ch->inLoadThread == 0){
            ch->inLoadThread = true;
            chunksToLoad.enqueue(ch);
        }
        else{
            std::cout << "ERROR: Tried to add chunk to load list and its in a thread! : " << (int)ch->inSaveThread << " " << (int)ch->inLoadThread << " " << ch->voxelPosition.z << std::endl;
        }
    }
    _cond.notify_one();
}

void ChunkIOManager::readWriteChunks()
{
    std::unique_lock<std::mutex> queueLock(_queueLock);
    Chunk *ch;

    nString reg;

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
            _isThreadFinished = 1;
            queueLock.unlock();
            return;
        }
        queueLock.unlock();

        // All tasks
        while (chunksToLoad.try_dequeue(ch) || chunksToSave.try_dequeue(ch)) {
            if (ch->getState() == ChunkStates::LOAD) {
                if (1 || _regionFileManager.tryLoadChunk(ch) == false) {
                    ch->loadStatus = 1;
                }

                finishedLoadChunks.enqueue(ch);
            } else { //save
               // _regionFileManager.saveChunk(ch);
                ch->inSaveThread = 0; //race condition?
            }
           
        }

        queueLock.lock();
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
}

bool ChunkIOManager::saveVersionFile() {
    return _regionFileManager.saveVersionFile();
}

bool ChunkIOManager::checkVersion() {
    return _regionFileManager.checkVersion();
}