#include "stdafx.h"
#include "ChunkMeshTask.h"

#include <Vorb/ThreadPool.h>

#include "BlockData.h"
#include "BlockPack.h"
#include "ChunkMeshManager.h"
#include "ChunkMesher.h"
#include "GameManager.h"
#include "Chunk.h"
#include "VoxelLightEngine.h"
#include "VoxelUtils.h"

void ChunkMeshTask::execute(WorkerData* workerData) {

    // Make the mesh!
    if (!chunk->hasCreatedMesh) {
        ChunkMeshMessage msg;
        msg.chunkID = chunk->getID();
        msg.data = &chunk->m_voxelPosition;
        msg.messageID = ChunkMeshMessageID::CREATE;

        meshManager->sendMessage(msg);
        chunk->hasCreatedMesh = true;
    }

    // Mesh updates are accompanied by light updates // TODO(Ben): Seems wasteful.
    if (workerData->voxelLightEngine == nullptr) {
        workerData->voxelLightEngine = new VoxelLightEngine();
    }
    
    // TODO(Ben): Lighting
    // updateLight(workerData->voxelLightEngine);
    
    // Lazily allocate chunkMesher // TODO(Ben): Seems wasteful.
    if (workerData->chunkMesher == nullptr) {
        workerData->chunkMesher = new ChunkMesher;
        workerData->chunkMesher->init(blockPack);
    }

    // Pre-processing
    workerData->chunkMesher->prepareDataAsync(chunk);
    

    ChunkMeshMessage msg;
    msg.messageID = ChunkMeshMessageID::UPDATE;
    msg.chunkID = chunk->getID();
    // We no longer care about chunk
    removeMeshDependencies(chunk);

    // Create the actual mesh
    msg.data = workerData->chunkMesher->createChunkMeshData(type);

    // Send it for update
    meshManager->sendMessage(msg);
}

void ChunkMeshTask::init(ChunkHandle ch, MeshTaskType cType, const BlockPack* blockPack, ChunkMeshManager* meshManager) {
    type = cType;
    chunk = ch;
    chunk->queuedForMesh = true;
    this->blockPack = blockPack;
    this->meshManager = meshManager;
}

void ChunkMeshTask::removeMeshDependencies(ChunkHandle chunk) {
#define RELEASE_HANDLE(a) tmp = a; tmp.release();

    ChunkHandle tmp;

    // Some tmp handles for less dereferencing
    ChunkHandle left = chunk->left;
    ChunkHandle right = chunk->right;
    ChunkHandle bottom = chunk->bottom;
    ChunkHandle top = chunk->top;
    ChunkHandle back = chunk->back;
    ChunkHandle front = chunk->front;
    ChunkHandle leftTop = left->top;
    ChunkHandle leftBot = left->bottom;
    ChunkHandle rightTop = right->top;
    ChunkHandle rightBot = right->bottom;

    // Remove dependencies
    RELEASE_HANDLE(left->back);
    RELEASE_HANDLE(left->front);
    RELEASE_HANDLE(leftTop->back);
    RELEASE_HANDLE(leftTop->front);
    RELEASE_HANDLE(leftTop);
    RELEASE_HANDLE(leftBot->back);
    RELEASE_HANDLE(leftBot->front);
    RELEASE_HANDLE(leftBot);
    RELEASE_HANDLE(right->back);
    RELEASE_HANDLE(right->front);
    RELEASE_HANDLE(rightTop->back);
    RELEASE_HANDLE(rightTop->front);
    RELEASE_HANDLE(rightTop);
    RELEASE_HANDLE(rightBot->back);
    RELEASE_HANDLE(rightBot->front);
    RELEASE_HANDLE(rightBot);
    RELEASE_HANDLE(top->back);
    RELEASE_HANDLE(top->front);
    RELEASE_HANDLE(bottom->back);
    RELEASE_HANDLE(bottom->front);
    chunk.release();
    left.release();
    right.release();
    bottom.release();
    top.release();
    back.release();
    front.release();
#undef RELEASE_HANDLE
}

// TODO(Ben): uhh
void ChunkMeshTask::updateLight(VoxelLightEngine* voxelLightEngine) {
    /* if (chunk->sunRemovalList.size()) {
         voxelLightEngine->calculateSunlightRemoval(chunk);
         }
         if (chunk->sunExtendList.size()) {
         voxelLightEngine->calculateSunlightExtend(chunk);
         }
         voxelLightEngine->calculateLight(chunk);*/
}
