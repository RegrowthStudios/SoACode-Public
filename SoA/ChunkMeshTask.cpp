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
    // Prepare message
    ChunkMeshMessage msg;
    msg.messageID = ChunkMeshMessageID::UPDATE;
    msg.chunkID = chunk->getID();

    // Pre-processing
    workerData->chunkMesher->prepareDataAsync(chunk, neighborHandles);

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
