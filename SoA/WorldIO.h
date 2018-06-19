#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <Windows.h>

#include <Vorb/types.h>
#include <zconf.h>

class Chunk;

const i32 SECTOR_SIZE = 512;

struct LocationBuffer {
public:
    ui8 buffer[16384];
    nString reg;
};

class WorldIO {
public:
    WorldIO();
    ~WorldIO();

    void addToSaveList(Chunk* ch);
    void addToSaveList(std::vector<Chunk*>& chunks);
    void addToLoadList(Chunk* ch);
    void addToLoadList(std::vector<Chunk*>& chunks);

    void beginThread();

    i32 isChunkSaved(Chunk* ch);
    void closeFile();

    void clearLoadList();
    i32 getLoadListSize();
    i32 getSaveListSize();

    void onQuit();

    void setDisableLoading(bool disableLoading) {
        _shouldDisableLoading = disableLoading;
    }

    std::queue<Chunk*> chunksToLoad;
    std::queue<Chunk*> chunksToSave; //store by region string
    std::thread* readWriteThread;
    std::mutex flcLock;
    std::vector<Chunk*> finishedLoadChunks;
private:
    i32 saveToFile(Chunk* ch);
    i32 loadFromFile(Chunk* ch);
    i32 deleteChunkFile(Chunk* ch);
    i32 fillChunkData(Chunk* ch);
    i32 tryReadFromFile();
    i32 openRegionFile(nString reg, i32 face, bool create, FILE** file, i32& fd);
    nString getRegionString(Chunk* ch);
    void compressBlockData(Chunk* ch);
    i32 seekToChunkOffset(Chunk* ch);
    i32 truncate(i64 size);
    ui32 getLocOffset(Chunk* ch);
    LocationBuffer* newLocationBuffer();

    void readWriteChunks(); //used by the thread

    std::mutex _queueLock;
    std::condition_variable _cond;

    FILE* _threadFile;
    i32 _threadFileDescriptors;
    nString _currReg;

#define CRW_BYTE_BUFSIZE 524288
    //These two buffers store the voxel data for block IDs and voxel light
    ui8 _byteBuffer[CRW_BYTE_BUFSIZE];
    uLongf _bufferSize;
    ui8 _compressedByteBuffer[CRW_BYTE_BUFSIZE + 12 + 52430];
    uLongf _compressedSize;

    i32 _maxLocationBufferCacheSize;
    LocationBuffer* _currLocationBuffer;
    std::queue<LocationBuffer*> _locationCacheQueue;
    std::map<nString, LocationBuffer*> _locationCache;
    std::map<nString, LocationBuffer*>::iterator _iterLocationCache;

    ui32 _chunkOffset, _chunkBlockSize; //the file location for the start of the chunk data
    ui32 _chunkLength, _oldChunkLength; //length of chunk data, old length of chunk data
    ui32 _locOffset;  //the file location for the start of the location data
    ui32 _fileSize;
    bool _isDirtyLocationBuffer;
    bool _isDone;
    bool _isThreadFinished;
    bool _shouldDisableLoading;
};