#include "stdafx.h"
#include "RenderTask.h"

#include <Vorb/ThreadPool.h>

#include "BlockData.h"
#include "BlockPack.h"
#include "ChunkMeshManager.h"
#include "ChunkMesher.h"
#include "GameManager.h"
#include "NChunk.h"
#include "VoxelLightEngine.h"
#include "VoxelUtils.h"

void RenderTask::execute(WorkerData* workerData) {

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

    // Mesh based on render job type
    switch (type) {
        case RenderTaskType::DEFAULT:
            workerData->chunkMesher->createChunkMesh(this);
            break;
        case RenderTaskType::LIQUID:
            workerData->chunkMesher->createOnlyWaterMesh(this);
            break;
    }
    ChunkMeshMessage msg;
    msg.chunkID = chunk->getID();
    msg.messageID = ChunkMeshMessageID::UPDATE;
    msg.data = workerData->chunkMesher->chunkMeshData;
    meshManager->sendMessage(msg);

    workerData->chunkMesher->chunkMeshData = nullptr;
}

void RenderTask::init(NChunk* ch, RenderTaskType cType, const BlockPack* blockPack, ChunkMeshManager* meshManager) {
    type = cType;
    chunk = ch;
    chunk->queuedForMesh = true;
    this->blockPack = blockPack;
    this->meshManager = meshManager;
}

// TODO(Ben): uhh
void RenderTask::updateLight(VoxelLightEngine* voxelLightEngine) {
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

void RenderTask::setupMeshData(ChunkMesher* chunkMesher) {
    int x, y, z, off1, off2;

    NChunk *ch1, *ch2;
    NChunk* left = chunk->left;
    NChunk* right = chunk->right;
    NChunk* bottom = chunk->bottom;
    NChunk* top = chunk->top;
    NChunk* back = chunk->back;
    NChunk* front = chunk->front;
    int wc;
    int c = 0;

    i32v3 pos;

    ui16* wvec = chunkMesher->_wvec;
    ui16* chData = chunkMesher->_blockIDData;
    ui16* chLampData = chunkMesher->_lampLightData;
    ui8* chSunlightData = chunkMesher->_sunlightData;
    ui16* chTertiaryData = chunkMesher->_tertiaryData;

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
    if (chunk->m_lamp.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
        //lamp data
        c = 0;
        auto& dataTree = chunk->m_lamp.getTree();
        for (int i = 0; i < dataTree.size(); i++) {
            for (int j = 0; j < dataTree[i].length; j++) {
                c = dataTree[i].getStart() + j;

                getPosFromBlockIndex(c, pos);
                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);

                chLampData[wc] = dataTree[i].data;
            }
        }
    } else {
        c = 0;
        for (y = 0; y < CHUNK_WIDTH; y++) {
            for (z = 0; z < CHUNK_WIDTH; z++) {
                for (x = 0; x < CHUNK_WIDTH; x++, c++) {
                    wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                    chLampData[wc] = chunk->m_lamp[c];
                }
            }
        }
    }
    if (chunk->m_sunlight.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
        //sunlight data
        c = 0;
        auto& dataTree = chunk->m_sunlight.getTree();
        for (int i = 0; i < dataTree.size(); i++) {
            for (int j = 0; j < dataTree[i].length; j++) {
                c = dataTree[i].getStart() + j;

                getPosFromBlockIndex(c, pos);
                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);

                chSunlightData[wc] = dataTree[i].data;
            }
        }
    } else {
        c = 0;
        for (y = 0; y < CHUNK_WIDTH; y++) {
            for (z = 0; z < CHUNK_WIDTH; z++) {
                for (x = 0; x < CHUNK_WIDTH; x++, c++) {
                    wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                    chSunlightData[wc] = chunk->m_sunlight[c];
                }
            }
        }
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
                    chTertiaryData[wc] = chunk->m_tertiary[c];
                }
            }
        }
    }
    chunk->unlock();

    //
    //bottom->lock();
    //top->lock();
    //for (z = 1; z < PADDED_WIDTH - 1; z++){
    //    for (x = 1; x < PADDED_WIDTH - 1; x++){
    //        off1 = (z - 1)*CHUNK_WIDTH + x - 1;
    //        off2 = z*PADDED_WIDTH + x;        

    //        //data
    //        chData[off2] = (bottom->getBlockData(CHUNK_SIZE - CHUNK_LAYER + off1)); //bottom
    //        chLampData[off2] = bottom->getLampLight(CHUNK_SIZE - CHUNK_LAYER + off1);
    //        chSunlightData[off2] = bottom->getSunlight(CHUNK_SIZE - CHUNK_LAYER + off1);
    //        chTertiaryData[off2] = bottom->getTertiaryData(CHUNK_SIZE - CHUNK_LAYER + off1);
    //        chData[off2 + PADDED_SIZE - PADDED_LAYER] = (top->getBlockData(off1)); //top
    //        chLampData[off2 + PADDED_SIZE - PADDED_LAYER] = top->getLampLight(off1);
    //        chSunlightData[off2 + PADDED_SIZE - PADDED_LAYER] = top->getSunlight(off1);
    //        chTertiaryData[off2 + PADDED_SIZE - PADDED_LAYER] = top->getTertiaryData(off1);
    //    }
    //}
    //top->unlock();
    //bottom->unlock();

    ////bottomleft
    //ch1 = bottom->left;
    //ch1->lock();
    //for (z = 1; z < PADDED_WIDTH - 1; z++){
    //    off2 = z*PADDED_WIDTH;            

    //    chData[off2] = (ch1->getBlockData(CHUNK_SIZE - CHUNK_LAYER + off1)); //bottom
    //    chLampData[off2] = ch1->getLampLight(CHUNK_SIZE - CHUNK_LAYER + off1);
    //    chSunlightData[off2] = ch1->getSunlight(CHUNK_SIZE - CHUNK_LAYER + off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(CHUNK_SIZE - CHUNK_LAYER + off1);
    //}
    //ch1->unlock();

    ////bottomleftback
    //ch2 = ch1->back;
    //off1 = CHUNK_SIZE - 1;
    //off2 = 0;
    //ch2->lock();
    //chData[off2] = (ch2->getBlockData(off1));
    //chLampData[off2] = ch2->getLampLight(off1);
    //chSunlightData[off2] = ch2->getSunlight(off1);
    //chTertiaryData[off2] = ch2->getTertiaryData(off1);
    //ch2->unlock();
    //
    ////bottomleftfront
    //ch2 = ch1->front;
    //off1 = CHUNK_SIZE - CHUNK_LAYER + CHUNK_WIDTH - 1;
    //off2 = PADDED_LAYER - PADDED_WIDTH;
    //ch2->lock();
    //chData[off2] = (ch2->getBlockData(off1));
    //chLampData[off2] = ch2->getLampLight(off1);
    //chSunlightData[off2] = ch2->getSunlight(off1);
    //chTertiaryData[off2] = ch2->getTertiaryData(off1);
    //ch2->unlock();
    //
    ////bottomright
    //ch1 = bottom->right;
    //ch1->lock();
    //for (z = 1; z < PADDED_WIDTH - 1; z++){
    //    off1 = CHUNK_SIZE - CHUNK_LAYER + (z - 1)*CHUNK_WIDTH;
    //    off2 = z*PADDED_WIDTH + PADDED_WIDTH - 1;        

    //    chData[off2] = (ch1->getBlockData(off1));
    //    chLampData[off2] = ch1->getLampLight(off1);
    //    chSunlightData[off2] = ch1->getSunlight(off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(off1);
    //}
    //ch1->unlock();

    ////bottomrightback
    //ch2 = ch1->back;
    //off1 = CHUNK_SIZE - CHUNK_WIDTH;
    //off2 = PADDED_WIDTH - 1;
    //ch2->lock();
    //chData[off2] = (ch2->getBlockData(off1));
    //chLampData[off2] = ch2->getLampLight(off1);
    //chSunlightData[off2] = ch2->getSunlight(off1);
    //chTertiaryData[off2] = ch2->getTertiaryData(off1);
    //ch2->unlock();
    //
    ////bottomrightfront
    //ch2 = ch1->front;
    //off1 = CHUNK_SIZE - CHUNK_LAYER;
    //off2 = PADDED_LAYER - 1;
    //ch2->lock();
    //chData[off2] = (ch2->getBlockData(off1));
    //chLampData[off2] = ch2->getLampLight(off1);
    //chSunlightData[off2] = ch2->getSunlight(off1);
    //chTertiaryData[off2] = ch2->getTertiaryData(off1);
    //ch2->unlock();
    //
    //

    ////backbottom
    //ch1 = back->bottom;
    //ch1->lock();
    //for (x = 1; x < PADDED_WIDTH - 1; x++) {
    //    off1 = CHUNK_SIZE - CHUNK_WIDTH + x - 1;
    //    off2 = x;

    //    chData[off2] = (ch1->getBlockData(off1));
    //    chLampData[off2] = ch1->getLampLight(off1);
    //    chSunlightData[off2] = ch1->getSunlight(off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(off1);
    //}
    //ch1->unlock();
    //


    ////frontbottom
    //ch1 = front->bottom;
    //ch1->lock();
    //for (x = 1; x < PADDED_WIDTH - 1; x++) {
    //    off1 = CHUNK_SIZE - CHUNK_LAYER + x - 1;
    //    off2 = PADDED_LAYER - PADDED_WIDTH + x;

    //    chData[off2] = (ch1->getBlockData(off1));
    //    chLampData[off2] = ch1->getLampLight(off1);
    //    chSunlightData[off2] = ch1->getSunlight(off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(off1);
    //}
    //ch1->unlock();
    //
    //left->lock();
    //right->lock();
    //for (y = 1; y < PADDED_WIDTH - 1; y++){
    //    for (z = 1; z < PADDED_WIDTH - 1; z++){
    //        off1 = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
    //        off2 = z*PADDED_WIDTH + y*PADDED_LAYER;

    //        chData[off2] = left->getBlockData(off1 + CHUNK_WIDTH - 1); //left
    //        chLampData[off2] = left->getLampLight(off1 + CHUNK_WIDTH - 1);
    //        chSunlightData[off2] = left->getSunlight(off1 + CHUNK_WIDTH - 1);
    //        chTertiaryData[off2] = left->getTertiaryData(off1 + CHUNK_WIDTH - 1);
    //        chData[off2 + PADDED_WIDTH - 1] = (right->getBlockData(off1));
    //        chLampData[off2 + PADDED_WIDTH - 1] = right->getLampLight(off1);
    //        chSunlightData[off2 + PADDED_WIDTH - 1] = right->getSunlight(off1);
    //        chTertiaryData[off2 + PADDED_WIDTH - 1] = right->getTertiaryData(off1);
    //    }
    //}
    //right->unlock();
    //left->unlock();

    //back->lock();
    //front->lock();
    //for (y = 1; y < PADDED_WIDTH - 1; y++) {
    //    for (x = 1; x < PADDED_WIDTH - 1; x++) {
    //        off1 = (x - 1) + (y - 1)*CHUNK_LAYER;
    //        off2 = x + y*PADDED_LAYER;

    //        chData[off2] = back->getBlockData(off1 + CHUNK_LAYER - CHUNK_WIDTH);
    //        chLampData[off2] = back->getLampLight(off1 + CHUNK_LAYER - CHUNK_WIDTH);
    //        chSunlightData[off2] = back->getSunlight(off1 + CHUNK_LAYER - CHUNK_WIDTH);
    //        chTertiaryData[off2] = back->getTertiaryData(off1 + CHUNK_LAYER - CHUNK_WIDTH);
    //        chData[off2 + PADDED_LAYER - PADDED_WIDTH] = (front->getBlockData(off1));
    //        chLampData[off2 + PADDED_LAYER - PADDED_WIDTH] = front->getLampLight(off1);
    //        chSunlightData[off2 + PADDED_LAYER - PADDED_WIDTH] = front->getSunlight(off1);
    //        chTertiaryData[off2 + PADDED_LAYER - PADDED_WIDTH] = front->getTertiaryData(off1);
    //    }
    //}
    //front->unlock();
    //back->unlock();

    //
    ////topleft
    //ch1 = top->left; 
    //ch1->lock();
    //for (z = 1; z < PADDED_WIDTH - 1; z++){
    //    off1 = z*CHUNK_WIDTH - 1;
    //    off2 = z*PADDED_WIDTH + PADDED_SIZE - PADDED_LAYER;
    //        
    //    chData[off2] = (ch1->getBlockData(off1));
    //    chLampData[off2] = ch1->getLampLight(off1);
    //    chSunlightData[off2] = ch1->getSunlight(off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(off1);
    //}
    //ch1->unlock();

    ////topleftback
    //ch2 = ch1->back;
    //if (ch2 && ch2->isAccessible) {
    //    off1 = CHUNK_LAYER - 1;
    //    off2 = PADDED_SIZE - PADDED_LAYER;
    //    ch2->lock();
    //    chData[off2] = (ch2->getBlockData(off1));
    //    chLampData[off2] = ch2->getLampLight(off1);
    //    chSunlightData[off2] = ch2->getSunlight(off1);
    //    chTertiaryData[off2] = ch2->getTertiaryData(off1);
    //    ch2->unlock();
    //}
    ////topleftfront
    //ch2 = ch1->front;
    //if (ch2 && ch2->isAccessible) {
    //    off1 = CHUNK_WIDTH - 1;
    //    off2 = PADDED_SIZE - PADDED_WIDTH;
    //    ch2->lock();
    //    chData[off2] = (ch2->getBlockData(off1));
    //    chLampData[off2] = ch2->getLampLight(off1);
    //    chSunlightData[off2] = ch2->getSunlight(off1);
    //    chTertiaryData[off2] = ch2->getTertiaryData(off1);
    //    ch2->unlock();
    //}
    //

    ////topright
    //ch1 = top->right;
    //ch1->lock();
    //for (z = 1; z < PADDED_WIDTH - 1; z++){
    //    off1 = (z - 1)*CHUNK_WIDTH;
    //    off2 = (z + 1)*PADDED_WIDTH - 1 + PADDED_SIZE - PADDED_LAYER;
    //    
    //    chData[off2] = (ch1->getBlockData(off1));
    //    chLampData[off2] = ch1->getLampLight(off1);
    //    chSunlightData[off2] = ch1->getSunlight(off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(off1);
    //}
    //ch1->unlock();

    ////toprightback
    //ch2 = ch1->back;
    //if (ch2 && ch2->isAccessible){
    //    off1 = CHUNK_LAYER - CHUNK_WIDTH;
    //    off2 = PADDED_SIZE - PADDED_LAYER + PADDED_WIDTH - 1;
    //    ch2->lock();
    //    chData[off2] = (ch2->getBlockData(off1));
    //    chLampData[off2] = ch2->getLampLight(off1);
    //    chSunlightData[off2] = ch2->getSunlight(off1);
    //    chTertiaryData[off2] = ch2->getTertiaryData(off1);
    //    ch2->unlock();
    //}
    ////toprightfront
    //ch2 = ch1->front;
    //if (ch2 && ch2->isAccessible){
    //    off1 = 0;
    //    off2 = PADDED_SIZE - 1;
    //    ch2->lock();
    //    chData[off2] = (ch2->getBlockData(off1));
    //    chLampData[off2] = ch2->getLampLight(off1);
    //    chSunlightData[off2] = ch2->getSunlight(off1);
    //    chTertiaryData[off2] = ch2->getTertiaryData(off1);
    //    ch2->unlock();
    //}
    //


    ////backtop
    //ch1 = back->top;
    //ch1->lock();
    //for (x = 1; x < PADDED_WIDTH - 1; x++) {
    //    off1 = CHUNK_LAYER - CHUNK_WIDTH + x - 1;
    //    off2 = PADDED_SIZE - PADDED_LAYER + x;

    //    chData[off2] = (ch1->getBlockData(off1));
    //    chLampData[off2] = ch1->getLampLight(off1);
    //    chSunlightData[off2] = ch1->getSunlight(off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(off1);
    //}
    //ch1->unlock();
    //
    //

    ////fronttop
    //ch1 = front->top;
    //ch1->lock();
    //for (x = 1; x < PADDED_WIDTH - 1; x++) {
    //    off1 = x - 1;
    //    off2 = PADDED_SIZE - PADDED_WIDTH + x;

    //    chData[off2] = (ch1->getBlockData(off1));
    //    chLampData[off2] = ch1->getLampLight(off1);
    //    chSunlightData[off2] = ch1->getSunlight(off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(off1);
    //}
    //ch1->unlock();
    //

    ////leftback
    //ch1 = left->back;
    //ch1->lock();
    //for (y = 1; y < PADDED_WIDTH - 1; y++) {
    //    off1 = y*CHUNK_LAYER - 1;
    //    off2 = y*PADDED_LAYER;

    //    chData[off2] = (ch1->getBlockData(off1));
    //    chLampData[off2] = ch1->getLampLight(off1);
    //    chSunlightData[off2] = ch1->getSunlight(off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(off1);
    //}
    //ch1->unlock();
    //
    //
    ////rightback
    //ch1 = right->back;
    //ch1->lock();
    //for (y = 1; y < PADDED_WIDTH - 1; y++) {
    //    off1 = y*CHUNK_LAYER - CHUNK_WIDTH;
    //    off2 = y*PADDED_LAYER + PADDED_WIDTH - 1;

    //    chData[off2] = (ch1->getBlockData(off1));
    //    chLampData[off2] = ch1->getLampLight(off1);
    //    chSunlightData[off2] = ch1->getSunlight(off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(off1);
    //}
    //ch1->unlock();
    //

    ////leftfront
    //ch1 = left->front;
    //ch1->lock();
    //for (y = 1; y < PADDED_WIDTH - 1; y++) {
    //    off1 = (y - 1)*CHUNK_LAYER + CHUNK_WIDTH - 1;
    //    off2 = (y + 1)*PADDED_LAYER - PADDED_WIDTH;

    //    chData[off2] = (ch1->getBlockData(off1));
    //    chLampData[off2] = ch1->getLampLight(off1);
    //    chSunlightData[off2] = ch1->getSunlight(off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(off1);
    //}
    //ch1->unlock();
    //

    ////rightfront
    //ch1 = right->front;
    //ch1->lock();
    //for (y = 1; y < PADDED_WIDTH - 1; y++) {
    //    off1 = (y - 1)*CHUNK_LAYER;
    //    off2 = (y + 1)*PADDED_LAYER - 1;

    //    chData[off2] = (ch1->getBlockData(off1));
    //    chLampData[off2] = ch1->getLampLight(off1);
    //    chSunlightData[off2] = ch1->getSunlight(off1);
    //    chTertiaryData[off2] = ch1->getTertiaryData(off1);
    //}
    //ch1->unlock();
}