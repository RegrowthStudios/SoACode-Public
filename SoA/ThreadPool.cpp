#include "stdafx.h"
#include "ThreadPool.h"
#include "GenerateTask.h"

#include "Errors.h"
#include "Chunk.h"
#include "ChunkMesher.h"
#include "WorldStructs.h"
#include "RenderTask.h"

ThreadPool::ThreadPool() {
    // Empty
}

ThreadPool::~ThreadPool() {
    destroy();
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

void ThreadPool::init(ui32 size) {
    // Check if its already initialized
    if (_isInitialized) return;
    _isInitialized = true;

    cout << "THREADPOOL SIZE: " << size << endl;

    /// Allocate all threads
    _workers.resize(size);
    for (ui32 i = 0; i < size; i++) {
        _workers[i] = new WorkerThread(&ThreadPool::workerThreadFunc, this);
    }
}

void ThreadPool::destroy() {
    if (!_isInitialized) return;
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
            _workers[i]->data.stop = true;
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
    std::vector<WorkerThread*>().swap(_workers);

    // We are no longer initialized
    _isInitialized = false;
}

bool ThreadPool::isFinished() {
    // Lock the mutex
    std::lock_guard<std::mutex> lock(_condMutex);
    // Check that the queue is empty
    if (_tasks.size_approx() != 0) return false;
    // Check that all workers are asleep
    for (size_t i = 0; i < _workers.size(); i++) {
        if (_workers[i]->data.waiting == false) {
            return false;
        }
    }
    return true;
}

void ThreadPool::workerThreadFunc(WorkerData* data) {
    data->waiting = false;
    data->stop = false;
    IThreadPoolTask* task;

    data->chunkMesher = new ChunkMesher;

    std::unique_lock<std::mutex> lock(_condMutex);
    while (true) {
        // Wait for work
        data->waiting = true;
        _cond.wait(lock);
        lock.unlock();
        // Check for exit
        if (data->stop) {
            delete data->chunkMesher;
            return;
        }
        // Grab a task if one exists
        std::cout << "TRY\n";
        if (_tasks.try_dequeue(task)) {
            std::cout << "DEQ\n";
            task->execute(data);
            task->setIsFinished(true);
            // Store result if needed
            if (1 || task->shouldAddToFinishedTasks()) {
                std::cout << "ADD\n";
                _finishedTasks.enqueue(task);
            }
        }

        // Re-lock the cond mutex so we can go to sleep
        lock.lock();
    }
}
