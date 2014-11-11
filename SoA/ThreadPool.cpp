#include "stdafx.h"
#include "ThreadPool.h"

#include "TaskQueueManager.h"
#include "Errors.h"
#include "Chunk.h"
#include "WorldStructs.h"
#include "RenderTask.h"

TaskQueueManager taskQueueManager;

ThreadPool::ThreadPool() {
    // Empty
}

ThreadPool::~ThreadPool() {
    if (_isInitialized == true && _isClosed == false) {
        close();
    }
}

void ThreadPool::clearTasks() {
    #define BATCH_SIZE 50
    IThreadPoolTask* tasks[BATCH_SIZE];
    int size;
    // Grab and kill tasks in batches
    while ((size = _tasks.try_dequeue_bulk(tasks, BATCH_SIZE))) {
        for (int i = 0; i < size; i++) {
            delete tasks[i];
        }
    }
}

void ThreadPool::addLoadJob(Chunk *chunk, LoadData *ld) {
    taskQueueManager.lock.lock();
    taskQueueManager.loadTaskQueue.push(LoadTask(chunk, ld));
    chunk->inGenerateThread = true;
    taskQueueManager.lock.unlock();
    taskQueueManager.cond.notify_one();
}

void ThreadPool::initialize(ui64 sz) {
    if (_isInitialized) return;
    _isInitialized = 1;
    _isClosed = false;
    for (int i = 0; i < 20; i++) {
        _workerData[i].chunkMesher = NULL;
    }
    if (sz > 20) sz = 20;
    _size = sz;
    for (Uint32 i = 0; i < 60; i++) {
        taskQueueManager.renderTaskPool.push_back(new RenderTask());
    }
    _workers.reserve(_size + 5);
    cout << "THREADPOOL SIZE: " << sz << endl;
    for (Uint32 i = 0; i < sz; i++) {
        _workers.push_back(new std::thread(WorkerThread, &(_workerData[i])));
    }
}

void ThreadPool::close() {
    // Clear all tasks
    clearTasks();
    // Wait for threads to all wait
    while (!(isFinished()));

    // Scope for lock_guard
    {
        // Lock the mutex
        std::lock_guard<std::mutex> lock(_condMutex);
        // Tell threads to quit
        for (size_t i = 0; i < _workers.size(); i++) {
            _workerData[i].stop = true;
        }
    }
    // Wake all threads so they can quit
    _cond.notify_all();
    // Join all threads
    for (size_t i = 0; i < _workers.size(); i++) {
        _workers[i]->join();
        delete _workers[i];
    }
    // Free memory
    std::vector<std::thread*>().swap(_workers);

    for (Uint32 i = 0; i < taskQueueManager.renderTaskPool.size(); i++) {
        delete taskQueueManager.renderTaskPool[i];
    }
    taskQueueManager.renderTaskPool.clear();

    _size = 0;
    _isInitialized = false;
    _isClosed = true;
}

void ThreadPool::addThread() {
    if (_size == 20) return;
    _workers.push_back(new std::thread(WorkerThread, &(_workerData[_size++])));
}

void ThreadPool::removeThread() {
   
}

int ThreadPool::addRenderJob(Chunk *chunk, MeshJobType type) {
    assert(chunk != nullptr);
    RenderTask *newRenderTask; //makes the task and allocates the memory for the buffers
    taskQueueManager.rpLock.lock();
    if (taskQueueManager.renderTaskPool.empty()) {
        taskQueueManager.rpLock.unlock();
        return 0;
    }
    newRenderTask = taskQueueManager.renderTaskPool.back();
    taskQueueManager.renderTaskPool.pop_back();
    taskQueueManager.rpLock.unlock(); //we can free lock while we work on data
    newRenderTask->setChunk(chunk, type);
    assert(newRenderTask->chunk != nullptr);
    chunk->SetupMeshData(newRenderTask);
    assert(newRenderTask->chunk != nullptr);

    chunk->inRenderThread = true;
    taskQueueManager.lock.lock();
    assert(newRenderTask->chunk != nullptr);
    taskQueueManager.renderTaskQueue.push(newRenderTask);
    taskQueueManager.lock.unlock();

    taskQueueManager.cond.notify_one();
    return 1;
}

bool ThreadPool::isFinished() {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(_condMutex);
    // Check that the queue is empty
    if (_tasks.size_approx() != 0) return false;
    // Check that all workers are asleep
    for (size_t i = 0; i < _workers.size(); i++) {
        if (_workerData[i].waiting == false) {
            return false;
        }
    }
    return true;
}

void ThreadPool::workerThreadFunc(WorkerData* data) {
    data->waiting = false;
    data->stop = false;
    IThreadPoolTask* task;

    std::unique_lock<std::mutex> lock(_condMutex);
    while (true) {
        // Wait for work
        data->waiting = true;
        _cond.wait(lock);
        lock.unlock();
        // Check for exit
        if (data->stop) return;

        // Grab a task if one exists
        if (_tasks.try_dequeue(task)) {
            task->execute();
            // Store result if needed
            if (task->shouldAddToFinishedTasks()) {
                _finishedTasks.enqueue(task);
            }
        }

        // Re-lock the cond mutex so we can go to sleep
        lock.lock();
    }
}
