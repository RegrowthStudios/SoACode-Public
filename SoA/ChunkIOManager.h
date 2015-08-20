#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include <ZLIB/zlib.h>

#include "RegionFileManager.h"
#include "readerwriterqueue.h"

class Chunk;

class ChunkIOManager{
public:
    ChunkIOManager(const nString& saveDir);
    ~ChunkIOManager();
    void clear();

    void addToSaveList(Chunk*  ch);
    void addToSaveList(std::vector<Chunk* >& chunks);
    void addToLoadList(Chunk*  ch);
    void addToLoadList(std::vector<Chunk* >& chunks);

    void beginThread();

    void onQuit();

    void setDisableLoading(bool disableLoading) { _shouldDisableLoading = disableLoading; }

    bool saveVersionFile();
    bool checkVersion();

    moodycamel::ReaderWriterQueue<Chunk* > chunksToLoad;
    moodycamel::ReaderWriterQueue<Chunk* > chunksToSave;
    std::thread* readWriteThread;
    moodycamel::ReaderWriterQueue<Chunk* > finishedLoadChunks;
private:
    RegionFileManager _regionFileManager;

    void readWriteChunks(); //used by the thread

    std::mutex _queueLock;
    std::condition_variable _cond;

    bool _isDone;
    bool _isThreadFinished;
    bool _shouldDisableLoading;
};