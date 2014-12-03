#include "stdafx.h"
#include "ThreadPool.h"

vcore::ThreadPool::ThreadPool() {
    // Empty
}

vcore::ThreadPool::~ThreadPool() {
    destroy();
}

void vcore::ThreadPool::clearTasks() {
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

void vcore::ThreadPool::init(ui32 size) {
    // Check if its already initialized
    if (_isInitialized) return;
    _isInitialized = true;

    std::cout << "THREADPOOL SIZE: " << size << std::endl;

    /// Allocate all threads
    _workers.resize(size);
    for (ui32 i = 0; i < size; i++) {
        _workers[i] = new WorkerThread(&ThreadPool::workerThreadFunc, this);
    }
}

void vcore::ThreadPool::destroy() {
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

bool vcore::ThreadPool::isFinished() {
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

void vcore::ThreadPool::workerThreadFunc(WorkerData* data) {
    data->waiting = false;
    data->stop = false;
    IThreadPoolTask* task;

    std::unique_lock<std::mutex> lock(_condMutex);
    lock.unlock();
    while (true) {
        // Check for exit
        if (data->stop) {
            return;
        }
        // Grab a task if one exists
        if (_tasks.try_dequeue(task)) {
            task->execute(data);
            task->setIsFinished(true);
            // Store result if needed
            if (task->shouldAddToFinishedTasks()) {
                _finishedTasks.enqueue(task);
            }
        } else {
            // Wait for work
            lock.lock();
            data->waiting = true;
            _cond.wait(lock);
            lock.unlock();
        }
    }
}
