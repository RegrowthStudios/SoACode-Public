#include "stdafx.h"
#include "RenderTask.h"

#include "Chunk.h"
#include "ChunkMesher.h"
#include "ThreadPool.h"

void RenderTask::execute(vcore::WorkerData* workerData) {
    switch (type) {
        case MeshJobType::DEFAULT:
            workerData->chunkMesher->createChunkMesh(this);
            break;
        case MeshJobType::LIQUID:
            workerData->chunkMesher->createOnlyWaterMesh(this);
            break;
    }
    // todo(Ben): Is this even needed?
    chunkMeshData = workerData->chunkMesher->chunkMeshData;
    workerData->chunkMesher->chunkMeshData = nullptr;
}

void RenderTask::setChunk(Chunk* ch, MeshJobType cType) {
    type = cType;
    chunk = ch;
    num = ch->numBlocks;
    position = ch->gridPosition;
    wSize = 0;
}