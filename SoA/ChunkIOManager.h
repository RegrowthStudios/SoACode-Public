#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <Windows.h>

#include <ZLIB/zlib.h>

#include "RegionFileManager.h"

class Chunk;

class ChunkIOManager {
public:
    ChunkIOManager();
    ~ChunkIOManager();
    void clear();

    void addToSaveList(Chunk* ch);
    void addToSaveList(std::vector<Chunk*>& chunks);
    void addToLoadList(Chunk* ch);
    void addToLoadList(std::vector<Chunk*>& chunks);

    void beginThread();

    i32 getLoadListSize();
    i32 getSaveListSize();

    void onQuit();

    void setDisableLoading(bool disableLoading) { _shouldDisableLoading = disableLoading; }

    bool saveVersionFile();
    bool checkVersion();

    std::queue<Chunk*> chunksToLoad;
    std::queue<Chunk*> chunksToSave; 
    std::thread* readWriteThread;
    std::mutex flcLock;
    std::vector<Chunk*> finishedLoadChunks;
private:
    RegionFileManager _regionFileManager;

    void readWriteChunks(); //used by the thread

    std::mutex _queueLock;
    std::condition_variable _cond;

    bool _isDone;
    bool _isThreadFinished;
    bool _shouldDisableLoading;
};