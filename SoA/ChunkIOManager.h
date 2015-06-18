#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include <ZLIB/zlib.h>

#include "RegionFileManager.h"
#include "readerwriterqueue.h"

class Chunk;

class ChunkIOManager {
public:
    ChunkIOManager(const nString& saveDir);
    ~ChunkIOManager();
    void clear();

    void addToSaveList(NChunk*  ch);
    void addToSaveList(std::vector<NChunk* >& chunks);
    void addToLoadList(NChunk*  ch);
    void addToLoadList(std::vector<NChunk* >& chunks);

    void beginThread();

    void onQuit();

    void setDisableLoading(bool disableLoading) { _shouldDisableLoading = disableLoading; }

    bool saveVersionFile();
    bool checkVersion();

    moodycamel::ReaderWriterQueue<NChunk* > chunksToLoad;
    moodycamel::ReaderWriterQueue<NChunk* > chunksToSave;
    std::thread* readWriteThread;
    moodycamel::ReaderWriterQueue<NChunk* > finishedLoadChunks;
private:
    RegionFileManager _regionFileManager;

    void readWriteChunks(); //used by the thread

    std::mutex _queueLock;
    std::condition_variable _cond;

    bool _isDone;
    bool _isThreadFinished;
    bool _shouldDisableLoading;
};