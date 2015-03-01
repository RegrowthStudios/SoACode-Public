#include "stdafx.h"
#include "RenderTask.h"

#include <Vorb/ThreadPool.h>

#include "Chunk.h"
#include "ChunkMesher.h"
#include "VoxelLightEngine.h"
#include "GameManager.h"
#include "ChunkMeshManager.h"

void RenderTask::execute(WorkerData* workerData) {

    // Mesh updates are accompanied by light updates
    if (workerData->voxelLightEngine == nullptr) {
        workerData->voxelLightEngine = new VoxelLightEngine();
    }
    updateLight(workerData->voxelLightEngine);
    // Lazily allocate chunkMesher
    if (workerData->chunkMesher == nullptr) {
        workerData->chunkMesher = new ChunkMesher;
    }

    // Pre-processing
    chunk->setupMeshData(workerData->chunkMesher);

    // Mesh based on render job type
    switch (type) {
        case RenderTaskType::DEFAULT:
            workerData->chunkMesher->createChunkMesh(this);
            break;
        case RenderTaskType::LIQUID:
            workerData->chunkMesher->createOnlyWaterMesh(this);
            break;
    }


    meshManager->addMeshForUpdate(workerData->chunkMesher->chunkMeshData);

    workerData->chunkMesher->chunkMeshData = nullptr;
}

void RenderTask::init(Chunk* ch, RenderTaskType cType, ChunkMeshManager* meshManager) {
    type = cType;
    chunk = ch;
    chunkMesh = chunk->mesh;
    this->meshManager = meshManager;
    chunkMesh->refCount++;
}

void RenderTask::updateLight(VoxelLightEngine* voxelLightEngine) {
    if (chunk->sunRemovalList.size()) {
        voxelLightEngine->calculateSunlightRemoval(chunk);
    }
    if (chunk->sunExtendList.size()) {
        voxelLightEngine->calculateSunlightExtend(chunk);
    }
    voxelLightEngine->calculateLight(chunk);
}
