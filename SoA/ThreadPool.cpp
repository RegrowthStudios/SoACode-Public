#include "stdafx.h"
#include "ThreadPool.h"

#include "TaskQueueManager.h"
#include "Errors.h"
#include "Chunk.h"
#include "WorldStructs.h"
#include "RenderTask.h"

TaskQueueManager taskQueueManager;

ThreadPool::ThreadPool() {
    _isClosed = 0;
    _isInitialized = 0;
    size = 0;
}

ThreadPool::~ThreadPool() {
    if (_isInitialized == 1 && _isClosed == 0) {
        taskQueueManager.lock.lock();
        for (size_t i = 0; i < _workers.size(); i++) {
            if (_workers[i]) {
                _workers[i]->detach();
                _workerData[i].stop = 1;
            }
        }
        taskQueueManager.lock.unlock();
        while (!isFinished()); //wait for all workers to finish
        taskQueueManager.cond.notify_all();

        for (size_t i = 0; i < _workers.size(); i++) {
            while (_workerData[i].finished != 1);
            if (_workers[i] != NULL) delete _workers[i];
        }
    }
}

void ThreadPool::clearJobs() {
    taskQueueManager.lock.lock();
    while (taskQueueManager.loadTaskQueue.size()) {
        delete taskQueueManager.loadTaskQueue.front().loadData;
        taskQueueManager.loadTaskQueue.pop();
    }
    queue<LoadTask>().swap(taskQueueManager.loadTaskQueue);
    queue<RenderTask*>().swap(taskQueueManager.renderTaskQueue);
    taskQueueManager.lock.unlock();
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
    _isClosed = 0;
    for (int i = 0; i < 20; i++) {
        _workerData[i].chunkMesher = NULL;
    }
    if (sz > 20) sz = 20;
    size = sz;
    for (Uint32 i = 0; i < 60; i++) {
        taskQueueManager.renderTaskPool.push_back(new RenderTask());
    }
    _workers.reserve(size + 5);
    cout << "THREADPOOL SIZE: " << sz << endl;
    for (Uint32 i = 0; i < sz; i++) {
        _workers.push_back(new std::thread(WorkerThread, &(_workerData[i])));
    }
}

void ThreadPool::close() {
    _isClosed = true;
    clearJobs();
    while (!(isFinished()));
    taskQueueManager.lock.lock();
    for (size_t i = 0; i < _workers.size(); i++) {
        _workers[i]->detach();
        _workerData[i].stop = 1;
    }
    taskQueueManager.lock.unlock();

    taskQueueManager.cond.notify_all();

    for (size_t i = 0; i < _workers.size();) {
        taskQueueManager.lock.lock();
        if (_workerData[i].finished == 1) {
            delete _workers[i];
            _workers[i] = NULL;
            i++;
        }
        taskQueueManager.lock.unlock();
    }

    for (Uint32 i = 0; i < taskQueueManager.renderTaskPool.size(); i++) {
        delete taskQueueManager.renderTaskPool[i];
    }
    taskQueueManager.renderTaskPool.clear();

    _workers.clear();
    size = 0;
    _isInitialized = false;
}

void ThreadPool::addThread() {
    if (size == 20) return;
    _workers.push_back(new std::thread(WorkerThread, &(_workerData[size++])));
}

void ThreadPool::removeThread() {
    size--;
    _workers[size]->detach();
    _workerData[size].stop = 1;
    taskQueueManager.cond.notify_all();
    taskQueueManager.lock.lock();
    while (_workerData[size].finished != 1) {
        taskQueueManager.lock.unlock(); taskQueueManager.lock.lock();
    }
    delete _workers[size];
    _workers.pop_back();
    taskQueueManager.lock.unlock();
}

int ThreadPool::addRenderJob(Chunk *chunk, MeshJobType type) {
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

    chunk->SetupMeshData(newRenderTask);

    chunk->inRenderThread = 1;
    taskQueueManager.lock.lock();
    taskQueueManager.renderTaskQueue.push(newRenderTask);
    taskQueueManager.lock.unlock();

    taskQueueManager.cond.notify_one();
    return 1;
}

bool ThreadPool::isFinished() {
    taskQueueManager.lock.lock();
    if (taskQueueManager.loadTaskQueue.size() != 0 || taskQueueManager.renderTaskQueue.size() != 0) {
        taskQueueManager.lock.unlock();
        taskQueueManager.lock.lock();
    }
    for (size_t i = 0; i < _workers.size(); i++) {
        if (_workerData[i].waiting == 0) {
            taskQueueManager.lock.unlock(); return 0;
        }
    }
    taskQueueManager.lock.unlock();
    return 1;
}