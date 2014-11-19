#include "stdafx.h"
#include "RenderTask.h"

#include "Chunk.h"
#include "ChunkMesher.h"
#include "ThreadPool.h"

void RenderTask::execute(vcore::WorkerData* workerData) {
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
    // TODO(Ben): Is this even needed?
    chunkMeshData = workerData->chunkMesher->chunkMeshData;
    workerData->chunkMesher->chunkMeshData = nullptr;
}

void RenderTask::init(Chunk* ch, RenderTaskType cType) {
    type = cType;
    chunk = ch;
}

void RenderTask::updateLight() {
    if (chunk->sunRemovalList.size()) {
        VoxelLightEngine::calculateSunlightRemoval(chunk);
    }
    if (chunk->sunExtendList.size()) {
        VoxelLightEngine::calculateSunlightExtend(chunk);
    }
    VoxelLightEngine::calculateLight(chunk);
}
