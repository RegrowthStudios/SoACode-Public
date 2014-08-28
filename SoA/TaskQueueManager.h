#pragma once
#include <mutex>
#include <queue>
#include <condition_variable>

#include "LoadTask.h"

struct RenderTask;
struct ChunkMeshData;

// Manages Task Queue
class TaskQueueManager {
public:
    std::mutex lock;
    std::condition_variable cond;
    std::condition_variable mainThreadCond;
    std::queue <LoadTask> loadTaskQueue;
    std::queue <RenderTask*> renderTaskQueue;

    std::mutex rpLock;
    std::vector <RenderTask*> renderTaskPool;
    std::mutex fcLock;
    std::vector <Chunk*> finishedChunks;
    std::mutex frLock;
    std::vector <ChunkMeshData*> finishedChunkMeshes;
};

// Singleton Instance
extern TaskQueueManager taskQueueManager;

