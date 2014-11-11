#pragma once
#include <thread>
#include <condition_variable>

#include "ChunkWorker.h"
#include "concurrentqueue.h"
#include "IThreadPoolTask.h"

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
    void clearTasks();
    void addLoadJob(Chunk* chunk, LoadData* ld);
    int addRenderJob(Chunk* chunk, MeshJobType type);
    bool isFinished();

    const i32& getSize() const { return _size; }

private:

    void workerThreadFunc(WorkerData* data);

    // Lock free task queues
    moodycamel::ConcurrentQueue<IThreadPoolTask*> _tasks;
    moodycamel::ConcurrentQueue<IThreadPoolTask*> _finishedTasks;

    std::mutex _condMutex; ///< Mutex for the conditional variable
    std::condition_variable _cond;

    // need to keep track of threads so we can join them
    bool _isInitialized = false;
    bool _isClosed = false;
    i32 _size = 0;
    std::vector<std::thread*> _workers;
    WorkerData _workerData[NUM_WORKER_CONTEXT]; // std::list?
};

