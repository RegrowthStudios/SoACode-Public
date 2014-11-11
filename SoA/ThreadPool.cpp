#include "stdafx.h"
#include "ThreadPool.h"

#include "TaskQueueManager.h"
#include "Errors.h"
#include "Chunk.h"
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

void ThreadPool::addLoadJob(Chunk *chunk, LoadData *ld) {
   
    LoadTask* task = new LoadTask(chunk, ld);
    chunk->inGenerateThread = true;

    taskQueueManager.cond.notify_one();
}

void ThreadPool::init(ui32 size) {
    // Check if its already initialized
    if (_isInitialized) return;
    _isInitialized = true;

    cout << "THREADPOOL SIZE: " << size << endl;

    /// Allocate all threads
    _workers.resize(size);
    for (ui32 i = 0; i < size; i++) {
        _workers[i] = new WorkerThread(&(this->workerThreadFunc));
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

    // Delete pool of render tasks
    for (Uint32 i = 0; i < taskQueueManager.renderTaskPool.size(); i++) {
        delete taskQueueManager.renderTaskPool[i];
    }
    taskQueueManager.renderTaskPool.clear();

    // We are no longer initialized
    _isInitialized = false;
}

int ThreadPool::addRenderJob(Chunk *chunk, MeshJobType type) {
    assert(chunk != nullptr);

    RenderTask *newRenderTask; //makes the task and allocates the memory for the buffers
  
    newRenderTask = new RenderTask;

    newRenderTask->setChunk(chunk, type);
  
    chunk->SetupMeshData(newRenderTask);

    chunk->inRenderThread = true;
    
    addTask(newRenderTask);

    _cond.notify_one();

    return 1;
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
