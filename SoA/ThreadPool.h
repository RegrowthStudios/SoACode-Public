#pragma once
#include <thread>

#include "ChunkWorker.h"

class Chunk;
struct LoadData;

#define NUM_WORKER_CONTEXT 20

enum class MeshJobType;

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    void initialize(ui64 sz);
    void close();
    void addThread();
    void removeThread();
    void clearJobs();
    void addLoadJob(Chunk* chunk, LoadData* ld);
    int addRenderJob(Chunk* chunk, MeshJobType type);
    bool isFinished();

    i32 size;
private:
    // need to keep track of threads so we can join them
    bool _isInitialized;
    bool _isClosed;
    std::vector<std::thread*> _workers;
    WorkerData _workerData[NUM_WORKER_CONTEXT];
};

