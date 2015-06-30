#include "stdafx.h"
#include "RenderTask.h"

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
    setupMeshData(workerData->chunkMesher);
    ChunkMeshMessage msg;
    msg.chunkID = chunk->getID();
    chunk->refCount--;

    // Mesh based on render job type
    switch (type) {
        case RenderTaskType::DEFAULT:
            workerData->chunkMesher->createChunkMesh(this);
            break;
        case RenderTaskType::LIQUID:
            workerData->chunkMesher->createOnlyWaterMesh(this);
            break;
    }
   
    
    msg.messageID = ChunkMeshMessageID::UPDATE;
    msg.data = workerData->chunkMesher->chunkMeshData;
    meshManager->sendMessage(msg);

    workerData->chunkMesher->chunkMeshData = nullptr;
}

void ChunkMeshTask::init(Chunk* ch, RenderTaskType cType, const BlockPack* blockPack, ChunkMeshManager* meshManager) {
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

// TODO(Ben): Temporary
#define GETBLOCK(a) blockPack->operator[](a)

void ChunkMeshTask::setupMeshData(ChunkMesher* chunkMesher) {
    int x, y, z, off1, off2;

    Chunk *ch1, *ch2;
    Chunk* left = chunk->left;
    Chunk* right = chunk->right;
    Chunk* bottom = chunk->bottom;
    Chunk* top = chunk->top;
    Chunk* back = chunk->back;
    Chunk* front = chunk->front;
    int wc;
    int c = 0;

    i32v3 pos;

    ui16* wvec = chunkMesher->m_wvec;
    ui16* chData = chunkMesher->blockData;
    ui16* chTertiaryData = chunkMesher->tertiaryData;

    memset(chData, 0, sizeof(chunkMesher->blockData));
    memset(chTertiaryData, 0, sizeof(chunkMesher->tertiaryData));

    chunkMesher->wSize = 0;
    chunkMesher->chunk = chunk;
    chunkMesher->chunkGridData = chunk->gridData;

    // TODO(Ben): Do this last so we can be queued for mesh longer?
    // TODO(Ben): Dude macro this or something.
    chunk->lock();
    chunk->queuedForMesh = false; ///< Indicate that we are no longer queued for a mesh
    if (chunk->m_blocks.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {

        int s = 0;
        //block data
        auto& dataTree = chunk->m_blocks.getTree();
        for (int i = 0; i < dataTree.size(); i++) {
            for (int j = 0; j < dataTree[i].length; j++) {
                c = dataTree[i].getStart() + j;

                getPosFromBlockIndex(c, pos);

                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);
                chData[wc] = dataTree[i].data;
                if (GETBLOCK(chData[wc]).meshType == MeshType::LIQUID) {
                    wvec[s++] = wc;
                }

            }
        }
        chunkMesher->wSize = s;
    } else {
        int s = 0;
        for (y = 0; y < CHUNK_WIDTH; y++) {
            for (z = 0; z < CHUNK_WIDTH; z++) {
                for (x = 0; x < CHUNK_WIDTH; x++, c++) {
                    wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                    chData[wc] = chunk->m_blocks[c];
                    if (GETBLOCK(chData[wc]).meshType == MeshType::LIQUID) {
                        wvec[s++] = wc;
                    }
                }
            }
        }
        chunkMesher->wSize = s;
    }
    if (chunk->m_tertiary.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
        //tertiary data
        c = 0;
        auto& dataTree = chunk->m_tertiary.getTree();
        for (int i = 0; i < dataTree.size(); i++) {
            for (int j = 0; j < dataTree[i].length; j++) {
                c = dataTree[i].getStart() + j;

                getPosFromBlockIndex(c, pos);
                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);

                chTertiaryData[wc] = dataTree[i].data;
            }
        }

    } else {
        c = 0;
        for (y = 0; y < CHUNK_WIDTH; y++) {
            for (z = 0; z < CHUNK_WIDTH; z++) {
                for (x = 0; x < CHUNK_WIDTH; x++, c++) {
                    wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                    chTertiaryData[wc] = chunk->m_tertiary.get(c);
                }
            }
        }
    }
    chunk->unlock();

    left->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        for (z = 1; z < PADDED_WIDTH - 1; z++) {
            off1 = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
            off2 = z*PADDED_WIDTH + y*PADDED_LAYER;

            chData[off2] = left->getBlockData(off1 + CHUNK_WIDTH - 1);
            chTertiaryData[off2] = left->getTertiaryData(off1 + CHUNK_WIDTH - 1);
        }
    }
    left->refCount--;
    left->unlock();

    right->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        for (z = 1; z < PADDED_WIDTH - 1; z++) {
            off1 = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
            off2 = z*PADDED_WIDTH + y*PADDED_LAYER;

            chData[off2 + PADDED_WIDTH - 1] = (right->getBlockData(off1));
            chTertiaryData[off2 + PADDED_WIDTH - 1] = right->getTertiaryData(off1);
        }
    }
    right->refCount--;
    right->unlock();


    bottom->lock();
    for (z = 1; z < PADDED_WIDTH - 1; z++) {
        for (x = 1; x < PADDED_WIDTH - 1; x++) {
            off1 = (z - 1)*CHUNK_WIDTH + x - 1;
            off2 = z*PADDED_WIDTH + x;
            //data
            chData[off2] = (bottom->getBlockData(CHUNK_SIZE - CHUNK_LAYER + off1)); //bottom
            chTertiaryData[off2] = bottom->getTertiaryData(CHUNK_SIZE - CHUNK_LAYER + off1);  
        }
    }
    bottom->refCount--;
    bottom->unlock();

    top->lock();
    for (z = 1; z < PADDED_WIDTH - 1; z++) {
        for (x = 1; x < PADDED_WIDTH - 1; x++) {
            off1 = (z - 1)*CHUNK_WIDTH + x - 1;
            off2 = z*PADDED_WIDTH + x;

            chData[off2 + PADDED_SIZE - PADDED_LAYER] = (top->getBlockData(off1)); //top
            chTertiaryData[off2 + PADDED_SIZE - PADDED_LAYER] = top->getTertiaryData(off1);
        }
    }
    top->refCount--;
    top->unlock();

    back->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        for (x = 1; x < PADDED_WIDTH - 1; x++) {
            off1 = (x - 1) + (y - 1)*CHUNK_LAYER;
            off2 = x + y*PADDED_LAYER;

            chData[off2] = back->getBlockData(off1 + CHUNK_LAYER - CHUNK_WIDTH);
            chTertiaryData[off2] = back->getTertiaryData(off1 + CHUNK_LAYER - CHUNK_WIDTH);
        }
    }
    back->refCount--;
    back->unlock();

    front->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        for (x = 1; x < PADDED_WIDTH - 1; x++) {
            off1 = (x - 1) + (y - 1)*CHUNK_LAYER;
            off2 = x + y*PADDED_LAYER;

            chData[off2 + PADDED_LAYER - PADDED_WIDTH] = (front->getBlockData(off1));
            chTertiaryData[off2 + PADDED_LAYER - PADDED_WIDTH] = front->getTertiaryData(off1);
        }
    }
    front->refCount--;
    front->unlock();

}