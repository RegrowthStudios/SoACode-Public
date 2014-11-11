#include "stdafx.h"
#include "ChunkWorker.h"

#include <mutex>

#include "Chunk.h"
#include "ChunkGenerator.h"
#include "ChunkMesher.h"
#include "RenderTask.h"
#include "TaskQueueManager.h"

void WorkerThreadOld(WorkerData *data) {
    // First Set Up The OpenGL Context
    data->chunkMesher = new ChunkMesher();

    data->waiting = 0;
    data->finished = 0;
    data->stop = 0;
    std::unique_lock<std::mutex> lock(taskQueueManager.lock);
    std::unique_lock<std::mutex> fcLock(taskQueueManager.fcLock);
    fcLock.unlock();
    std::unique_lock<std::mutex> frLock(taskQueueManager.frLock);
    frLock.unlock();
    std::unique_lock<std::mutex> rpLock(taskQueueManager.rpLock);
    rpLock.unlock();

    LoadTask loadTask;
    RenderTask* renderTask;
    Chunk* chunk = nullptr;

    while(1) {
        while(taskQueueManager.loadTaskQueue.empty() && taskQueueManager.renderTaskQueue.empty()) { //thread might wake up only to need to go right back to sleep
            data->chunkMesher->freeBuffers();
            data->waiting = true;
            //taskQueueManager.mainThreadCond.notify_one();
            if(data->stop) {
                if(data->chunkMesher) delete data->chunkMesher;
                data->finished = true;
                lock.unlock();
                return;
            }
            taskQueueManager.cond.wait(lock);
            data->waiting = 0;
        }
        if(data->stop) {
            if(data->chunkMesher) delete data->chunkMesher;
            data->finished = true;
            lock.unlock();
            return;
        }
        if(taskQueueManager.loadTaskQueue.size()) {
            loadTask = taskQueueManager.loadTaskQueue.front();
            chunk = loadTask.chunk;
            taskQueueManager.loadTaskQueue.pop();
            lock.unlock();
            ChunkGenerator::generateChunk(chunk, loadTask.loadData);
            delete loadTask.loadData;

            fcLock.lock();
            chunk->inFinishedChunks = true;
            taskQueueManager.finishedChunks.push_back(chunk);
            fcLock.unlock();
        }
        else { //loading gets priority
            renderTask = taskQueueManager.renderTaskQueue.front();
            chunk = renderTask->chunk;
            taskQueueManager.renderTaskQueue.pop();
            lock.unlock();

            switch (renderTask->type) {
                case MeshJobType::DEFAULT:
                    data->chunkMesher->createChunkMesh(renderTask);
                    break;
                case MeshJobType::LIQUID:
                    data->chunkMesher->createOnlyWaterMesh(renderTask);
                    break;
            }

			//return to the cache/pool
			rpLock.lock();
			taskQueueManager.renderTaskPool.push_back(renderTask);
			rpLock.unlock();
			frLock.lock();
            assert(chunk != nullptr);
            if (!chunk) {
                std::cout << "Loading Task Chunk Is Null\n";
            } else {
                chunk->inFinishedMeshes = 1;
            }
			taskQueueManager.finishedChunkMeshes.push_back(data->chunkMesher->chunkMeshData);
			data->chunkMesher->chunkMeshData = NULL;
			frLock.unlock();
		}
		lock.lock();
	}
}