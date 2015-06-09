#include "stdafx.h"
#include "Chunk.h"

#include <Vorb/ThreadPool.h>
#include <Vorb/utils.h>

#include "Biome.h"
#include "BlockData.h"
#include "BlockPack.h"
#include "CAEngine.h"
#include "ChunkMesher.h"
#include "Errors.h"
#include "Frustum.h"
#include "GameManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "RenderTask.h"
#include "Rendering.h"
#include "SimplexNoise.h"
#include "SphericalTerrainGpuGenerator.h"
#include "TerrainGenerator.h"
#include "VoxelUtils.h"
#include "readerwriterqueue.h"

GLuint Chunk::vboIndicesID = 0;

std::vector<MineralData*> Chunk::possibleMinerals;

void Chunk::init(const ChunkPosition3D &chunkPos) {
	loadStatus = 0;
	freeWaiting = 0;
    isAccessible = false;
    inLoadThread = false;
    inSaveThread = false;
    queuedForMesh = false;
	dirty = 0;
	
    _chunkListPtr = NULL;
	setupWaitingTime = 0;
	treeTryTicks = 0;

    voxelPosition = chunkPos.pos * CHUNK_WIDTH;
  
	numBlocks = -1;
	_state = ChunkStates::LOAD;
	left = NULL;
	right = NULL;
	back = NULL;
	top = NULL;
	bottom = NULL;
	front = NULL;
    needsNeighbors = false;
    numNeighbors = 0;
    distance2 = 999999.0;
	treesToLoad.clear();
	blockUpdateIndex = 0;
    _levelOfDetail = 1;
    meshJobCounter = 0;
    chunkDependencies = 0;
    
    for (size_t i = 0; i < blockUpdateList.size(); i++) {
		blockUpdateList[i].clear();
	}

	spawnerBlocks.clear();
	drawWater = 0;
    lastOwnerTask = nullptr;

    gridPosition = chunkPos;

    mesh = nullptr;
}

void Chunk::clear(bool clearDraw)
{
    freeWaiting = false;
    _blockIDContainer.clear();
    _lampLightContainer.clear();
    _sunlightContainer.clear();
    _tertiaryDataContainer.clear();

    _state = ChunkStates::LOAD;
    isAccessible = false;
    left = right = front = back = top = bottom = nullptr;
    _chunkListPtr = nullptr;
    treeTryTicks = 0;

    std::vector<ui16>().swap(spawnerBlocks);
    std::vector<TreeData>().swap(treesToLoad);
    std::vector<PlantData>().swap(plantsToLoad);
    std::vector<ui16>().swap(sunRemovalList);
    std::vector<ui16>().swap(sunExtendList);

    for (size_t i = 0; i < blockUpdateList.size(); i++) {
        std::vector <ui16>().swap(blockUpdateList[i]); //release the memory manually
    }
    std::queue<LampLightRemovalNode>().swap(lampLightRemovalQueue);
    std::queue<LampLightUpdateNode>().swap(lampLightUpdateQueue);
    std::queue<SunlightRemovalNode>().swap(sunlightRemovalQueue);
    std::queue<SunlightUpdateNode>().swap(sunlightUpdateQueue);
    if (clearDraw){
        clearBuffers();
    }
}

void Chunk::clearBuffers()
{
	if (mesh){
        // Will signify that it needs to be destroyed in render thread
        mesh->needsDestroy = true;
        mesh = nullptr;
	}
}

void Chunk::clearNeighbors() {
    if (left) {
        left->right = nullptr;
        left->numNeighbors--;
    }
    if (right) {
        right->left = nullptr;
        right->numNeighbors--;
    }
    if (top) {
        top->bottom = nullptr;
        top->numNeighbors--;
    }
    if (bottom) {
        bottom->top = nullptr;
        bottom->numNeighbors--;
    }
    if (front) {
        front->back = nullptr;
        front->numNeighbors--;
    }
    if (back) {
        back->front = nullptr;
        back->numNeighbors--;
    }
    numNeighbors = 0;
    left = right = top = bottom = back = front = nullptr;
}

int Chunk::GetPlantType(int x, int z, Biome *biome)
{
 /*   double typer;
    NoiseInfo *nf;
    for (Uint32 i = 0; i < biome->possibleFlora.size(); i++){
        typer = PseudoRand(x + i*(z + 555) + gridPosition.x, z - i*(x + 666) + gridPosition.z) + 1.0;
        nf = GameManager::planet->floraNoiseFunctions[biome->possibleFlora[i].floraIndex];
        if (nf != NULL){
            if (typer < (biome->possibleFlora[i].probability*scaled_octave_noise_2d(nf->octaves, nf->persistence, nf->frequency, nf->lowBound, nf->upBound, x + i * 6666, z - i * 5555))){
                return biome->possibleFlora[i].floraIndex;
            }
        }
        else{
            if (typer < (biome->possibleFlora[i].probability)){
                return biome->possibleFlora[i].floraIndex;
            }
        }
    }*/
    return NONE; //default
}

// Used for flood fill occlusion testing. Currently not used.
void Chunk::CheckEdgeBlocks()
{
    //int x, y, z;
    //topBlocked = leftBlocked = rightBlocked = bottomBlocked = frontBlocked = backBlocked = 1;
    ////top
    //y = CHUNK_WIDTH - 1;
    //for (x = 0; x < CHUNK_WIDTH; x++){
    //    for (z = 0; z < CHUNK_WIDTH; z++){
    //        if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
    //            topBlocked = 0;
    //            z = CHUNK_WIDTH;
    //            x = CHUNK_WIDTH;
    //        }
    //    }
    //}
    ////left
    //x = 0;
    //for (y = 0; y < CHUNK_WIDTH; y++){
    //    for (z = 0; z < CHUNK_WIDTH; z++){
    //        if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
    //            leftBlocked = 0;
    //            z = CHUNK_WIDTH;
    //            y = CHUNK_WIDTH;
    //        }
    //    }
    //}
    ////right
    //x = CHUNK_WIDTH - 1;
    //for (y = 0; y < CHUNK_WIDTH; y++){
    //    for (z = 0; z < CHUNK_WIDTH; z++){
    //        if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
    //            rightBlocked = 0;
    //            z = CHUNK_WIDTH;
    //            y = CHUNK_WIDTH;
    //        }
    //    }
    //}

    ////bottom
    //y = 0;
    //for (x = 0; x < CHUNK_WIDTH; x++){
    //    for (z = 0; z < CHUNK_WIDTH; z++){
    //        if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
    //            bottomBlocked = 0;
    //            z = CHUNK_WIDTH;
    //            x = CHUNK_WIDTH;
    //        }
    //    }
    //}

    ////front
    //z = CHUNK_WIDTH - 1;
    //for (x = 0; x < CHUNK_WIDTH; x++){
    //    for (y = 0; y < CHUNK_WIDTH; y++){
    //        if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
    //            frontBlocked = 0;
    //            y = CHUNK_WIDTH;
    //            x = CHUNK_WIDTH;
    //        }
    //    }
    //}

    ////back
    //z = 0;
    //for (x = 0; x < CHUNK_WIDTH; x++){
    //    for (y = 0; y < CHUNK_WIDTH; y++){
    //        if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
    //            backBlocked = 0;
    //            y = CHUNK_WIDTH;
    //            x = CHUNK_WIDTH;
    //        }
    //    }
    //}
}

void Chunk::setupMeshData(ChunkMesher* chunkMesher) {
    int x, y, z, off1, off2;

    Chunk *ch1, *ch2;
    int wc;
    int c = 0;

    i32v3 pos;

    ui16* wvec = chunkMesher->_wvec;
    ui16* chData = chunkMesher->_blockIDData;
    ui16* chLampData = chunkMesher->_lampLightData;
    ui8* chSunlightData = chunkMesher->_sunlightData;
    ui16* chTertiaryData = chunkMesher->_tertiaryData;

    chunkMesher->wSize = 0;
    chunkMesher->chunk = this;
    chunkMesher->chunkGridData = chunkGridData;

    lock();
    queuedForMesh = false; ///< Indicate that we are no longer queued for a mesh
    if (_blockIDContainer.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {

        int s = 0;
        //block data
        auto& dataTree = _blockIDContainer.getTree();
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
                    chData[wc] = _blockIDContainer[c];
                    if (GETBLOCK(chData[wc]).meshType == MeshType::LIQUID) {
                        wvec[s++] = wc;
                    }
                }
            }
        }
        chunkMesher->wSize = s;
    }
    if (_lampLightContainer.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
        //lamp data
        c = 0;
        auto& dataTree = _lampLightContainer.getTree();
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
                    chLampData[wc] = _lampLightContainer[c];
                }
            }
        }
    }
    if (_sunlightContainer.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
        //sunlight data
        c = 0;
        auto& dataTree = _sunlightContainer.getTree();
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
                    chSunlightData[wc] = _sunlightContainer[c];
                }
            }
        }
    }
    if (_tertiaryDataContainer.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
        //tertiary data
        c = 0;
        auto& dataTree = _tertiaryDataContainer.getTree();
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
                    chTertiaryData[wc] = _tertiaryDataContainer[c];
                }
            }
        }
    }
    unlock();
 
    
    bottom->lock();
    top->lock();
    for (z = 1; z < PADDED_WIDTH - 1; z++){
        for (x = 1; x < PADDED_WIDTH - 1; x++){
            off1 = (z - 1)*CHUNK_WIDTH + x - 1;
            off2 = z*PADDED_WIDTH + x;        

            //data
            chData[off2] = (bottom->getBlockData(CHUNK_SIZE - CHUNK_LAYER + off1)); //bottom
            chLampData[off2] = bottom->getLampLight(CHUNK_SIZE - CHUNK_LAYER + off1);
            chSunlightData[off2] = bottom->getSunlight(CHUNK_SIZE - CHUNK_LAYER + off1);
            chTertiaryData[off2] = bottom->getTertiaryData(CHUNK_SIZE - CHUNK_LAYER + off1);
            chData[off2 + PADDED_SIZE - PADDED_LAYER] = (top->getBlockData(off1)); //top
            chLampData[off2 + PADDED_SIZE - PADDED_LAYER] = top->getLampLight(off1);
            chSunlightData[off2 + PADDED_SIZE - PADDED_LAYER] = top->getSunlight(off1);
            chTertiaryData[off2 + PADDED_SIZE - PADDED_LAYER] = top->getTertiaryData(off1);
        }
    }
    top->unlock();
    bottom->unlock();
   
    //bottomleft
    ch1 = bottom->left;
    ch1->lock();
    for (z = 1; z < PADDED_WIDTH - 1; z++){
        off2 = z*PADDED_WIDTH;            

        chData[off2] = (ch1->getBlockData(CHUNK_SIZE - CHUNK_LAYER + off1)); //bottom
        chLampData[off2] = ch1->getLampLight(CHUNK_SIZE - CHUNK_LAYER + off1);
        chSunlightData[off2] = ch1->getSunlight(CHUNK_SIZE - CHUNK_LAYER + off1);
        chTertiaryData[off2] = ch1->getTertiaryData(CHUNK_SIZE - CHUNK_LAYER + off1);
    }
    ch1->unlock();

    //bottomleftback
    ch2 = ch1->back;
    off1 = CHUNK_SIZE - 1;
    off2 = 0;
    ch2->lock();
    chData[off2] = (ch2->getBlockData(off1));
    chLampData[off2] = ch2->getLampLight(off1);
    chSunlightData[off2] = ch2->getSunlight(off1);
    chTertiaryData[off2] = ch2->getTertiaryData(off1);
    ch2->unlock();
    

    //bottomleftfront
    ch2 = ch1->front;
    off1 = CHUNK_SIZE - CHUNK_LAYER + CHUNK_WIDTH - 1;
    off2 = PADDED_LAYER - PADDED_WIDTH;
    ch2->lock();
    chData[off2] = (ch2->getBlockData(off1));
    chLampData[off2] = ch2->getLampLight(off1);
    chSunlightData[off2] = ch2->getSunlight(off1);
    chTertiaryData[off2] = ch2->getTertiaryData(off1);
    ch2->unlock();
    
   

    //bottomright
    ch1 = bottom->right;
    ch1->lock();
    for (z = 1; z < PADDED_WIDTH - 1; z++){
        off1 = CHUNK_SIZE - CHUNK_LAYER + (z - 1)*CHUNK_WIDTH;
        off2 = z*PADDED_WIDTH + PADDED_WIDTH - 1;        

        chData[off2] = (ch1->getBlockData(off1));
        chLampData[off2] = ch1->getLampLight(off1);
        chSunlightData[off2] = ch1->getSunlight(off1);
        chTertiaryData[off2] = ch1->getTertiaryData(off1);
    }
    ch1->unlock();

    //bottomrightback
    ch2 = ch1->back;
    off1 = CHUNK_SIZE - CHUNK_WIDTH;
    off2 = PADDED_WIDTH - 1;
    ch2->lock();
    chData[off2] = (ch2->getBlockData(off1));
    chLampData[off2] = ch2->getLampLight(off1);
    chSunlightData[off2] = ch2->getSunlight(off1);
    chTertiaryData[off2] = ch2->getTertiaryData(off1);
    ch2->unlock();
    
    //bottomrightfront
    ch2 = ch1->front;
    off1 = CHUNK_SIZE - CHUNK_LAYER;
    off2 = PADDED_LAYER - 1;
    ch2->lock();
    chData[off2] = (ch2->getBlockData(off1));
    chLampData[off2] = ch2->getLampLight(off1);
    chSunlightData[off2] = ch2->getSunlight(off1);
    chTertiaryData[off2] = ch2->getTertiaryData(off1);
    ch2->unlock();
    
    

    //backbottom
    ch1 = back->bottom;
    ch1->lock();
    for (x = 1; x < PADDED_WIDTH - 1; x++) {
        off1 = CHUNK_SIZE - CHUNK_WIDTH + x - 1;
        off2 = x;

        chData[off2] = (ch1->getBlockData(off1));
        chLampData[off2] = ch1->getLampLight(off1);
        chSunlightData[off2] = ch1->getSunlight(off1);
        chTertiaryData[off2] = ch1->getTertiaryData(off1);
    }
    ch1->unlock();
    


    //frontbottom
    ch1 = front->bottom;
    ch1->lock();
    for (x = 1; x < PADDED_WIDTH - 1; x++) {
        off1 = CHUNK_SIZE - CHUNK_LAYER + x - 1;
        off2 = PADDED_LAYER - PADDED_WIDTH + x;

        chData[off2] = (ch1->getBlockData(off1));
        chLampData[off2] = ch1->getLampLight(off1);
        chSunlightData[off2] = ch1->getSunlight(off1);
        chTertiaryData[off2] = ch1->getTertiaryData(off1);
    }
    ch1->unlock();
    
    left->lock();
    right->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++){
        for (z = 1; z < PADDED_WIDTH - 1; z++){
            off1 = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
            off2 = z*PADDED_WIDTH + y*PADDED_LAYER;

            chData[off2] = left->getBlockData(off1 + CHUNK_WIDTH - 1); //left
            chLampData[off2] = left->getLampLight(off1 + CHUNK_WIDTH - 1);
            chSunlightData[off2] = left->getSunlight(off1 + CHUNK_WIDTH - 1);
            chTertiaryData[off2] = left->getTertiaryData(off1 + CHUNK_WIDTH - 1);
            chData[off2 + PADDED_WIDTH - 1] = (right->getBlockData(off1));
            chLampData[off2 + PADDED_WIDTH - 1] = right->getLampLight(off1);
            chSunlightData[off2 + PADDED_WIDTH - 1] = right->getSunlight(off1);
            chTertiaryData[off2 + PADDED_WIDTH - 1] = right->getTertiaryData(off1);
        }
    }
    right->unlock();
    left->unlock();
   
    back->lock();
    front->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        for (x = 1; x < PADDED_WIDTH - 1; x++) {
            off1 = (x - 1) + (y - 1)*CHUNK_LAYER;
            off2 = x + y*PADDED_LAYER;

            chData[off2] = back->getBlockData(off1 + CHUNK_LAYER - CHUNK_WIDTH);
            chLampData[off2] = back->getLampLight(off1 + CHUNK_LAYER - CHUNK_WIDTH);
            chSunlightData[off2] = back->getSunlight(off1 + CHUNK_LAYER - CHUNK_WIDTH);
            chTertiaryData[off2] = back->getTertiaryData(off1 + CHUNK_LAYER - CHUNK_WIDTH);
            chData[off2 + PADDED_LAYER - PADDED_WIDTH] = (front->getBlockData(off1));
            chLampData[off2 + PADDED_LAYER - PADDED_WIDTH] = front->getLampLight(off1);
            chSunlightData[off2 + PADDED_LAYER - PADDED_WIDTH] = front->getSunlight(off1);
            chTertiaryData[off2 + PADDED_LAYER - PADDED_WIDTH] = front->getTertiaryData(off1);
        }
    }
    front->unlock();
    back->unlock();
  
    
    //topleft
    ch1 = top->left; 
    ch1->lock();
    for (z = 1; z < PADDED_WIDTH - 1; z++){
        off1 = z*CHUNK_WIDTH - 1;
        off2 = z*PADDED_WIDTH + PADDED_SIZE - PADDED_LAYER;
            
        chData[off2] = (ch1->getBlockData(off1));
        chLampData[off2] = ch1->getLampLight(off1);
        chSunlightData[off2] = ch1->getSunlight(off1);
        chTertiaryData[off2] = ch1->getTertiaryData(off1);
    }
    ch1->unlock();

    //topleftback
    ch2 = ch1->back;
    if (ch2 && ch2->isAccessible) {
        off1 = CHUNK_LAYER - 1;
        off2 = PADDED_SIZE - PADDED_LAYER;
        ch2->lock();
        chData[off2] = (ch2->getBlockData(off1));
        chLampData[off2] = ch2->getLampLight(off1);
        chSunlightData[off2] = ch2->getSunlight(off1);
        chTertiaryData[off2] = ch2->getTertiaryData(off1);
        ch2->unlock();
    }
    //topleftfront
    ch2 = ch1->front;
    if (ch2 && ch2->isAccessible) {
        off1 = CHUNK_WIDTH - 1;
        off2 = PADDED_SIZE - PADDED_WIDTH;
        ch2->lock();
        chData[off2] = (ch2->getBlockData(off1));
        chLampData[off2] = ch2->getLampLight(off1);
        chSunlightData[off2] = ch2->getSunlight(off1);
        chTertiaryData[off2] = ch2->getTertiaryData(off1);
        ch2->unlock();
    }
    

    //topright
    ch1 = top->right;
    ch1->lock();
    for (z = 1; z < PADDED_WIDTH - 1; z++){
        off1 = (z - 1)*CHUNK_WIDTH;
        off2 = (z + 1)*PADDED_WIDTH - 1 + PADDED_SIZE - PADDED_LAYER;
        
        chData[off2] = (ch1->getBlockData(off1));
        chLampData[off2] = ch1->getLampLight(off1);
        chSunlightData[off2] = ch1->getSunlight(off1);
        chTertiaryData[off2] = ch1->getTertiaryData(off1);
    }
    ch1->unlock();

    //toprightback
    ch2 = ch1->back;
    if (ch2 && ch2->isAccessible){
        off1 = CHUNK_LAYER - CHUNK_WIDTH;
        off2 = PADDED_SIZE - PADDED_LAYER + PADDED_WIDTH - 1;
        ch2->lock();
        chData[off2] = (ch2->getBlockData(off1));
        chLampData[off2] = ch2->getLampLight(off1);
        chSunlightData[off2] = ch2->getSunlight(off1);
        chTertiaryData[off2] = ch2->getTertiaryData(off1);
        ch2->unlock();
    }
    //toprightfront
    ch2 = ch1->front;
    if (ch2 && ch2->isAccessible){
        off1 = 0;
        off2 = PADDED_SIZE - 1;
        ch2->lock();
        chData[off2] = (ch2->getBlockData(off1));
        chLampData[off2] = ch2->getLampLight(off1);
        chSunlightData[off2] = ch2->getSunlight(off1);
        chTertiaryData[off2] = ch2->getTertiaryData(off1);
        ch2->unlock();
    }
    


    //backtop
    ch1 = back->top;
    ch1->lock();
    for (x = 1; x < PADDED_WIDTH - 1; x++) {
        off1 = CHUNK_LAYER - CHUNK_WIDTH + x - 1;
        off2 = PADDED_SIZE - PADDED_LAYER + x;

        chData[off2] = (ch1->getBlockData(off1));
        chLampData[off2] = ch1->getLampLight(off1);
        chSunlightData[off2] = ch1->getSunlight(off1);
        chTertiaryData[off2] = ch1->getTertiaryData(off1);
    }
    ch1->unlock();
    
    

    //fronttop
    ch1 = front->top;
    ch1->lock();
    for (x = 1; x < PADDED_WIDTH - 1; x++) {
        off1 = x - 1;
        off2 = PADDED_SIZE - PADDED_WIDTH + x;

        chData[off2] = (ch1->getBlockData(off1));
        chLampData[off2] = ch1->getLampLight(off1);
        chSunlightData[off2] = ch1->getSunlight(off1);
        chTertiaryData[off2] = ch1->getTertiaryData(off1);
    }
    ch1->unlock();
    

    //leftback
    ch1 = left->back;
    ch1->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        off1 = y*CHUNK_LAYER - 1;
        off2 = y*PADDED_LAYER;

        chData[off2] = (ch1->getBlockData(off1));
        chLampData[off2] = ch1->getLampLight(off1);
        chSunlightData[off2] = ch1->getSunlight(off1);
        chTertiaryData[off2] = ch1->getTertiaryData(off1);
    }
    ch1->unlock();
    
    
    //rightback
    ch1 = right->back;
    ch1->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        off1 = y*CHUNK_LAYER - CHUNK_WIDTH;
        off2 = y*PADDED_LAYER + PADDED_WIDTH - 1;

        chData[off2] = (ch1->getBlockData(off1));
        chLampData[off2] = ch1->getLampLight(off1);
        chSunlightData[off2] = ch1->getSunlight(off1);
        chTertiaryData[off2] = ch1->getTertiaryData(off1);
    }
    ch1->unlock();
    

    //leftfront
    ch1 = left->front;
    ch1->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        off1 = (y - 1)*CHUNK_LAYER + CHUNK_WIDTH - 1;
        off2 = (y + 1)*PADDED_LAYER - PADDED_WIDTH;

        chData[off2] = (ch1->getBlockData(off1));
        chLampData[off2] = ch1->getLampLight(off1);
        chSunlightData[off2] = ch1->getSunlight(off1);
        chTertiaryData[off2] = ch1->getTertiaryData(off1);
    }
    ch1->unlock();
    

    //rightfront
    ch1 = right->front;
    ch1->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        off1 = (y - 1)*CHUNK_LAYER;
        off2 = (y + 1)*PADDED_LAYER - 1;

        chData[off2] = (ch1->getBlockData(off1));
        chLampData[off2] = ch1->getLampLight(off1);
        chSunlightData[off2] = ch1->getSunlight(off1);
        chTertiaryData[off2] = ch1->getTertiaryData(off1);
    }
    ch1->unlock();
}

void Chunk::addToChunkList(std::vector<Chunk*> *chunkListPtr) {
    _chunkListPtr = chunkListPtr;
    chunkListPtr->push_back(this);
}

void Chunk::clearChunkListPtr() {
    _chunkListPtr = nullptr;
}

bool Chunk::hasCaUpdates(const std::vector <CaPhysicsType*>& typesToUpdate) {
    for (auto& type : typesToUpdate) {
        const int& caIndex = type->getCaIndex();
        if (!blockUpdateList[caIndex * 2 + (int)activeUpdateList[caIndex]].empty()) {
            return true;
        }
    }
    return false;
}

int Chunk::getRainfall(int xz) const {
    return (int)chunkGridData->heightData[xz].rainfall;
}

int Chunk::getTemperature(int xz) const {
    return (int)chunkGridData->heightData[xz].temperature;
}

void Chunk::detectNeighbors(const std::unordered_map<i32v3, Chunk*>& chunkMap) {

    std::unordered_map<i32v3, Chunk*>::const_iterator it;

    //left
    if (left == nullptr) {
        it = chunkMap.find(gridPosition.pos + i32v3(-1, 0, 0));
        if (it != chunkMap.end()) {
            left = it->second;
            left->right = this;
            numNeighbors++;
            left->numNeighbors++;
        }
    }
    //right
    if (right == nullptr) {
        it = chunkMap.find(gridPosition.pos + i32v3(1, 0, 0));
        if (it != chunkMap.end()) {
            right = it->second;
            right->left = this;
            numNeighbors++;
            right->numNeighbors++;
        }
    }

    //back
    if (back == nullptr) {
        it = chunkMap.find(gridPosition.pos + i32v3(0, 0, -1));
        if (it != chunkMap.end()) {
            back = it->second;
            back->front = this;
            numNeighbors++;
            back->numNeighbors++;
        }
    }

    //front
    if (front == nullptr) {
        it = chunkMap.find(gridPosition.pos + i32v3(0, 0, 1));
        if (it != chunkMap.end()) {
            front = it->second;
            front->back = this;
            numNeighbors++;
            front->numNeighbors++;
        }
    }

    //bottom
    if (bottom == nullptr) {
        it = chunkMap.find(gridPosition.pos + i32v3(0, -1, 0));
        if (it != chunkMap.end()) {
            bottom = it->second;
            bottom->top = this;
            numNeighbors++;
            bottom->numNeighbors++;
        }
    }

    //top
    if (top == nullptr) {
        it = chunkMap.find(gridPosition.pos + i32v3(0, 1, 0));
        if (it != chunkMap.end()) {
            top = it->second;
            top->bottom = this;
            numNeighbors++;
            top->numNeighbors++;
        }
    }
}

double Chunk::getDistance2(const i32v3& pos, const i32v3& cameraPos) {
    double dx = (cameraPos.x <= pos.x) ? pos.x : ((cameraPos.x > pos.x + CHUNK_WIDTH) ? (pos.x + CHUNK_WIDTH) : cameraPos.x);
    double dy = (cameraPos.y <= pos.y) ? pos.y : ((cameraPos.y > pos.y + CHUNK_WIDTH) ? (pos.y + CHUNK_WIDTH) : cameraPos.y);
    double dz = (cameraPos.z <= pos.z) ? pos.z : ((cameraPos.z > pos.z + CHUNK_WIDTH) ? (pos.z + CHUNK_WIDTH) : cameraPos.z);
    dx = dx - cameraPos.x;
    dy = dy - cameraPos.y;
    dz = dz - cameraPos.z;
    //we dont sqrt the distance since sqrt is slow
    return dx*dx + dy*dy + dz*dz;
}