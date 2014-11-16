#include "stdafx.h"
#include "Chunk.h"

#include <boost\circular_buffer.hpp>

#include "BlockData.h"
#include "ChunkMesher.h"
#include "Errors.h"
#include "Frustum.h"
#include "GameManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "Planet.h"
#include "readerwriterqueue.h"
#include "RenderTask.h"
#include "Rendering.h"
#include "SimplexNoise.h"
#include "Sound.h"
#include "TerrainGenerator.h"
#include "ThreadPool.h"
#include "MessageManager.h"
#include "utils.h"
#include "VoxelUtils.h"

GLuint Chunk::vboIndicesID = 0;

vector <MineralData*> Chunk::possibleMinerals;

glm::mat4 MVP;
glm::mat4 GlobalModelMatrix;

//1735
//1500
double surfaceDensity[9][5][5];

void Chunk::init(const i32v3 &gridPos, ChunkSlot* Owner){

	topBlocked = leftBlocked = rightBlocked = bottomBlocked = frontBlocked = backBlocked = 0;
	loadStatus = 0;
	freeWaiting = 0;
	hasLoadedSunlight = 0;
	isAccessible = 0;
	inLoadThread = 0;
	inSaveThread = 0;
	dirty = 0;
	//THIS MUST COME BEFORE CLEARBUFFERS
	mesh = NULL;
	clearBuffers();
	_chunkListPtr = NULL;
	setupWaitingTime = 0;
	treeTryTicks = 0;
    gridPosition = gridPos;
    chunkPosition.x = fastFloor(gridPosition.x / (double)CHUNK_WIDTH);
    chunkPosition.y = fastFloor(gridPosition.y / (double)CHUNK_WIDTH);
    chunkPosition.z = fastFloor(gridPosition.z / (double)CHUNK_WIDTH);
	numBlocks = -1;
	_state = ChunkStates::LOAD;
	left = NULL;
	right = NULL;
	back = NULL;
	top = NULL;
	bottom = NULL;
	front = NULL;
	numNeighbors = 0;
	distance2 = 999999.0;
	treesToLoad.clear();
	blockUpdateIndex = 0;
    _levelOfDetail = 1;
    
	for (int i = 0; i < 8; i++){
		blockUpdateList[i][0].clear();
		blockUpdateList[i][1].clear();
		activeUpdateList[i] = 0;
	}

	spawnerBlocks.clear();
	drawWater = 0;
	occlude = 0;
    owner = Owner;
    lastOwnerTask = nullptr;
    distance2 = owner->distance2;
    chunkGridData = owner->chunkGridData;
    voxelMapData = chunkGridData->voxelMapData;
}

vector <Chunk*> *dbgst;

void Chunk::clear(bool clearDraw)
{
    clearBuffers();
    freeWaiting = false;
    voxelMapData = nullptr;
    _blockIDContainer.clear();
    _lampLightContainer.clear();
    _sunlightContainer.clear();
    _tertiaryDataContainer.clear();

    _state = ChunkStates::LOAD;
    isAccessible = 0;
    left = right = front = back = top = bottom = nullptr;
    _chunkListPtr = nullptr;
    treeTryTicks = 0;

    vector<ui16>().swap(spawnerBlocks);
    vector<TreeData>().swap(treesToLoad);
    vector<PlantData>().swap(plantsToLoad);
    vector<ui16>().swap(sunRemovalList);
    vector<ui16>().swap(sunExtendList); 

    for (int i = 0; i < 8; i++){
        vector <GLushort>().swap(blockUpdateList[i][0]); //release the memory manually
        vector <GLushort>().swap(blockUpdateList[i][1]);
    }
    vector<LampLightRemovalNode>().swap(lampLightRemovalQueue);
    vector<LampLightUpdateNode>().swap(lampLightUpdateQueue);
    vector<SunlightRemovalNode>().swap(sunlightRemovalQueue);
    vector<SunlightUpdateNode>().swap(sunlightUpdateQueue);
    if (clearDraw){
        clearBuffers();
    }
}

void Chunk::clearBuffers()
{
	if (mesh){
		ChunkMeshData *cmd = new ChunkMeshData(this);
		cmd->chunkMesh = mesh;
		mesh = NULL;

        GameManager::messageManager->enqueue(ThreadId::UPDATE,
                                             Message(MessageID::CHUNK_MESH, 
                                             (void*)cmd));

	}
}

void Chunk::clearNeighbors() {
    if (left && left->right == this) {
        left->right = nullptr;
        left->numNeighbors--;
    }
    if (right && right->left == this) {
        right->left = nullptr;
        right->numNeighbors--;
    }
    if (top && top->bottom == this) {
        top->bottom = nullptr;
        top->numNeighbors--;
    }
    if (bottom && bottom->top == this) {
        bottom->top = nullptr;
        bottom->numNeighbors--;
    }
    if (front && front->back == this) {
        front->back = nullptr;
        front->numNeighbors--;
    }
    if (back && back->front == this) {
        back->front = nullptr;
        back->numNeighbors--;
    }
    numNeighbors = 0;
    left = right = top = bottom = back = front = nullptr;
}

int Chunk::GetPlantType(int x, int z, Biome *biome)
{
    double typer;
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
    }
    return NONE; //default
}

// Used for flood fill occlusion testing. Currently not used.
void Chunk::CheckEdgeBlocks()
{
    int x, y, z;
    topBlocked = leftBlocked = rightBlocked = bottomBlocked = frontBlocked = backBlocked = 1;
    //top
    y = CHUNK_WIDTH - 1;
    for (x = 0; x < CHUNK_WIDTH; x++){
        for (z = 0; z < CHUNK_WIDTH; z++){
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
                topBlocked = 0;
                z = CHUNK_WIDTH;
                x = CHUNK_WIDTH;
            }
        }
    }
    //left
    x = 0;
    for (y = 0; y < CHUNK_WIDTH; y++){
        for (z = 0; z < CHUNK_WIDTH; z++){
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
                leftBlocked = 0;
                z = CHUNK_WIDTH;
                y = CHUNK_WIDTH;
            }
        }
    }
    //right
    x = CHUNK_WIDTH - 1;
    for (y = 0; y < CHUNK_WIDTH; y++){
        for (z = 0; z < CHUNK_WIDTH; z++){
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
                rightBlocked = 0;
                z = CHUNK_WIDTH;
                y = CHUNK_WIDTH;
            }
        }
    }

    //bottom
    y = 0;
    for (x = 0; x < CHUNK_WIDTH; x++){
        for (z = 0; z < CHUNK_WIDTH; z++){
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
                bottomBlocked = 0;
                z = CHUNK_WIDTH;
                x = CHUNK_WIDTH;
            }
        }
    }

    //front
    z = CHUNK_WIDTH - 1;
    for (x = 0; x < CHUNK_WIDTH; x++){
        for (y = 0; y < CHUNK_WIDTH; y++){
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
                frontBlocked = 0;
                y = CHUNK_WIDTH;
                x = CHUNK_WIDTH;
            }
        }
    }

    //back
    z = 0;
    for (x = 0; x < CHUNK_WIDTH; x++){
        for (y = 0; y < CHUNK_WIDTH; y++){
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == BlockOcclusion::NONE){
                backBlocked = 0;
                y = CHUNK_WIDTH;
                x = CHUNK_WIDTH;
            }
        }
    }
}

void Chunk::setupMeshData(ChunkMesher* chunkMesher)
{
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

    //Must have all neighbors
    assert(top && left && right && back && front && bottom);

    lock();

    if (_blockIDContainer.getState() == vvoxel::VoxelStorageState::INTERVAL_TREE) {

        int s = 0;
        //block data
        for (int i = 0; i < _blockIDContainer._dataTree.size(); i++) {
            for (int j = 0; j < _blockIDContainer._dataTree[i].length; j++) {
                c = _blockIDContainer._dataTree[i].getStart() + j;
                assert(c < CHUNK_SIZE);
                getPosFromBlockIndex(c, pos);

                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);
                chData[wc] = _blockIDContainer._dataTree[i].data;
                if (GETBLOCK(chData[wc]).physicsProperty == PhysicsProperties::P_LIQUID) {
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
                    chData[wc] = _blockIDContainer._dataArray[c];
                    if (GETBLOCK(chData[wc]).physicsProperty == PhysicsProperties::P_LIQUID) {
                        wvec[s++] = wc;
                    }
                }
            }
        }
        chunkMesher->wSize = s;
    }
    if (_lampLightContainer.getState() == vvoxel::VoxelStorageState::INTERVAL_TREE) {
        //lamp data
        c = 0;
        for (int i = 0; i < _lampLightContainer._dataTree.size(); i++) {
            for (int j = 0; j < _lampLightContainer._dataTree[i].length; j++) {
                c = _lampLightContainer._dataTree[i].getStart() + j;

                assert(c < CHUNK_SIZE);
                getPosFromBlockIndex(c, pos);
                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);

                chLampData[wc] = _lampLightContainer._dataTree[i].data;
            }
        }
    } else {
        c = 0;
        for (y = 0; y < CHUNK_WIDTH; y++) {
            for (z = 0; z < CHUNK_WIDTH; z++) {
                for (x = 0; x < CHUNK_WIDTH; x++, c++) {
                    wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                    chLampData[wc] = _lampLightContainer._dataArray[c];
                }
            }
        }
    }
    if (_sunlightContainer.getState() == vvoxel::VoxelStorageState::INTERVAL_TREE) {
        //sunlight data
        c = 0;
        for (int i = 0; i < _sunlightContainer._dataTree.size(); i++) {
            for (int j = 0; j < _sunlightContainer._dataTree[i].length; j++) {
                c = _sunlightContainer._dataTree[i].getStart() + j;

                assert(c < CHUNK_SIZE);
                getPosFromBlockIndex(c, pos);
                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);

                chSunlightData[wc] = _sunlightContainer._dataTree[i].data;
            }
        }
    } else {
        c = 0;
        for (y = 0; y < CHUNK_WIDTH; y++) {
            for (z = 0; z < CHUNK_WIDTH; z++) {
                for (x = 0; x < CHUNK_WIDTH; x++, c++) {
                    wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                    chSunlightData[wc] = _sunlightContainer._dataArray[c];
                }
            }
        }
    }
    if (_tertiaryDataContainer.getState() == vvoxel::VoxelStorageState::INTERVAL_TREE) {
        //tertiary data
        c = 0;
        for (int i = 0; i < _tertiaryDataContainer._dataTree.size(); i++) {
            for (int j = 0; j < _tertiaryDataContainer._dataTree[i].length; j++) {
                c = _tertiaryDataContainer._dataTree[i].getStart() + j;

                assert(c < CHUNK_SIZE);
                getPosFromBlockIndex(c, pos);
                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);

                chTertiaryData[wc] = _tertiaryDataContainer._dataTree[i].data;
            }
        }

    } else {
        c = 0;
        for (y = 0; y < CHUNK_WIDTH; y++) {
            for (z = 0; z < CHUNK_WIDTH; z++) {
                for (x = 0; x < CHUNK_WIDTH; x++, c++) {
                    wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                    chTertiaryData[wc] = _tertiaryDataContainer._dataArray[c];
                }
            }
        }
    }
    unlock();
 
    //top and bottom
    ch1 = bottom;
    ch2 = top;

    if (ch1 && ch2 && ch1->isAccessible && ch2->isAccessible){
        ch1->lock();
        ch2->lock();
        for (z = 1; z < PADDED_WIDTH - 1; z++){
            for (x = 1; x < PADDED_WIDTH - 1; x++){
                off1 = (z - 1)*CHUNK_WIDTH + x - 1;
                off2 = z*PADDED_WIDTH + x;        

                //data
                chData[off2] = (ch1->getBlockData(CHUNK_SIZE - CHUNK_LAYER + off1)); //bottom
                chLampData[off2] = ch1->getLampLight(CHUNK_SIZE - CHUNK_LAYER + off1);
                chSunlightData[off2] = ch1->getSunlight(CHUNK_SIZE - CHUNK_LAYER + off1);
                chTertiaryData[off2] = ch1->getTertiaryData(CHUNK_SIZE - CHUNK_LAYER + off1);
                chData[off2 + PADDED_SIZE - PADDED_LAYER] = (ch2->getBlockData(off1)); //top
                chLampData[off2 + PADDED_SIZE - PADDED_LAYER] = ch2->getLampLight(off1);
                chSunlightData[off2 + PADDED_SIZE - PADDED_LAYER] = ch2->getSunlight(off1);
                chTertiaryData[off2 + PADDED_SIZE - PADDED_LAYER] = ch2->getTertiaryData(off1);
            }
        }
        ch2->unlock();
        ch1->unlock();
    }
    else{
        for (z = 1; z < PADDED_WIDTH - 1; z++){
            for (x = 1; x < PADDED_WIDTH - 1; x++){
                off1 = (z - 1)*CHUNK_WIDTH + x - 1;
                off2 = z*PADDED_WIDTH + x;
            
                chLampData[off2] = 0;
                chSunlightData[off2] = 0;
                chTertiaryData[off2] = 0;
                chLampData[off2 + PADDED_SIZE - PADDED_LAYER] = 0;
                chSunlightData[off2 + PADDED_SIZE - PADDED_LAYER] = 0;
                chTertiaryData[off2 + PADDED_SIZE - PADDED_LAYER] = 0;
                chData[off2 + PADDED_SIZE - PADDED_LAYER] = 0;
                chData[off2] = 0;
            }
        }
    }
    //bottomleft
    ch1 = bottom->left;
    if (ch1 && ch1->isAccessible){
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
        if (ch2 && ch2->isAccessible) {
            off1 = CHUNK_SIZE - 1;
            off2 = 0;    
            ch2->lock();
            chData[off2] = (ch2->getBlockData(off1));
            chLampData[off2] = ch2->getLampLight(off1);
            chSunlightData[off2] = ch2->getSunlight(off1);
            chTertiaryData[off2] = ch2->getTertiaryData(off1);
            ch2->unlock();
        }

        //bottomleftfront
        ch2 = ch1->front;
        if (ch2 && ch2->isAccessible) {
            off1 = CHUNK_SIZE - CHUNK_LAYER + CHUNK_WIDTH - 1;
            off2 = PADDED_LAYER - PADDED_WIDTH;    
            ch2->lock();
            chData[off2] = (ch2->getBlockData(off1));
            chLampData[off2] = ch2->getLampLight(off1);
            chSunlightData[off2] = ch2->getSunlight(off1);
            chTertiaryData[off2] = ch2->getTertiaryData(off1);
            ch2->unlock();
        }
    }
    else{
        for (z = 1; z < PADDED_WIDTH - 1; z++){
            chData[z*PADDED_WIDTH] = 0;
            chLampData[z*PADDED_WIDTH] = 0;
            chSunlightData[z*PADDED_WIDTH] = 0;
            chTertiaryData[z*PADDED_WIDTH] = 0;
        }

        chData[0] = 0;
        chLampData[0] = 0;
        chSunlightData[0] = 0;
        chTertiaryData[0] = 0;
        chData[PADDED_LAYER - PADDED_WIDTH] = 0;
        chLampData[PADDED_LAYER - PADDED_WIDTH] = 0;
        chSunlightData[PADDED_LAYER - PADDED_WIDTH] = 0;
        chTertiaryData[PADDED_LAYER - PADDED_WIDTH] = 0;
    }

    //bottomright
    ch1 = bottom->right;
    if (ch1 && ch1->isAccessible){
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
        if (ch2 && ch2->isAccessible) {
            off1 = CHUNK_SIZE - CHUNK_WIDTH;
            off2 = PADDED_WIDTH - 1;
            ch2->lock();
            chData[off2] = (ch2->getBlockData(off1));
            chLampData[off2] = ch2->getLampLight(off1);
            chSunlightData[off2] = ch2->getSunlight(off1);
            chTertiaryData[off2] = ch2->getTertiaryData(off1);
            ch2->unlock();
        }
        //bottomrightfront
        ch2 = ch1->front;
        if (ch2 && ch2->isAccessible) {
            off1 = CHUNK_SIZE - CHUNK_LAYER;
            off2 = PADDED_LAYER - 1;
            ch2->lock();
            chData[off2] = (ch2->getBlockData(off1));
            chLampData[off2] = ch2->getLampLight(off1);
            chSunlightData[off2] = ch2->getSunlight(off1);
            chTertiaryData[off2] = ch2->getTertiaryData(off1);
            ch2->unlock();
        }
    }

    //backbottom
    ch1 = back->bottom;
    if (ch1 && ch1->isAccessible){
        ch1->lock();
        for (x = 1; x < PADDED_WIDTH - 1; x++){
            off1 = CHUNK_SIZE - CHUNK_WIDTH + x - 1;
            off2 = x;

            chData[off2] = (ch1->getBlockData(off1));
            chLampData[off2] = ch1->getLampLight(off1);
            chSunlightData[off2] = ch1->getSunlight(off1);
            chTertiaryData[off2] = ch1->getTertiaryData(off1);
        }
        ch1->unlock();
    }


    //frontbottom
    ch1 = front->bottom;
    if (ch1 && ch1->isAccessible){
        ch1->lock();
        for (x = 1; x < PADDED_WIDTH - 1; x++){
            off1 = CHUNK_SIZE - CHUNK_LAYER + x - 1;
            off2 = PADDED_LAYER - PADDED_WIDTH + x;
        
            chData[off2] = (ch1->getBlockData(off1));
            chLampData[off2] = ch1->getLampLight(off1);
            chSunlightData[off2] = ch1->getSunlight(off1);
            chTertiaryData[off2] = ch1->getTertiaryData(off1);
        }
        ch1->unlock();
    }


    //left and right
    ch1 = left;
    ch2 = right;
    if (ch1 && ch2 && ch1->isAccessible && ch2->isAccessible){
        ch1->lock();
        ch2->lock();
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            for (z = 1; z < PADDED_WIDTH - 1; z++){
                off1 = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
                off2 = z*PADDED_WIDTH + y*PADDED_LAYER;

                chData[off2] = ch1->getBlockData(off1 + CHUNK_WIDTH - 1); //left
                chLampData[off2] = ch1->getLampLight(off1 + CHUNK_WIDTH - 1);
                chSunlightData[off2] = ch1->getSunlight(off1 + CHUNK_WIDTH - 1);
                chTertiaryData[off2] = ch1->getTertiaryData(off1 + CHUNK_WIDTH - 1);
                chData[off2 + PADDED_WIDTH - 1] = (ch2->getBlockData(off1));
                chLampData[off2 + PADDED_WIDTH - 1] = ch2->getLampLight(off1);
                chSunlightData[off2 + PADDED_WIDTH - 1] = ch2->getSunlight(off1);
                chTertiaryData[off2 + PADDED_WIDTH - 1] = ch2->getTertiaryData(off1);
            }
        }
        ch2->unlock();
        ch1->unlock();
    }
    else{
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            for (z = 1; z < PADDED_WIDTH - 1; z++){
                off1 = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
                off2 = z*PADDED_WIDTH + y*PADDED_LAYER;
            
                chLampData[off2] = 0;
                chSunlightData[off2] = 0;
                chTertiaryData[off2] = 0;
                chLampData[off2 + PADDED_WIDTH - 1] = 0;
                chSunlightData[off2 + PADDED_WIDTH - 1] = 0;
                chTertiaryData[off2 + PADDED_WIDTH - 1] = 0;
                chData[off2 + PADDED_WIDTH - 1] = 0;
                chData[off2] = 0;
            }
        }
    }

    //front and back
    ch1 = back;
    ch2 = front;
    if (ch1 && ch2 && ch1->isAccessible && ch2->isAccessible){
        ch1->lock();
        ch2->lock();
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            for (x = 1; x < PADDED_WIDTH - 1; x++){
                off1 = (x - 1) + (y - 1)*CHUNK_LAYER;
                off2 = x + y*PADDED_LAYER;
            
                chData[off2] = ch1->getBlockData(off1 + CHUNK_LAYER - CHUNK_WIDTH);
                chLampData[off2] = ch1->getLampLight(off1 + CHUNK_LAYER - CHUNK_WIDTH);
                chSunlightData[off2] = ch1->getSunlight(off1 + CHUNK_LAYER - CHUNK_WIDTH);
                chTertiaryData[off2] = ch1->getTertiaryData(off1 + CHUNK_LAYER - CHUNK_WIDTH);
                chData[off2 + PADDED_LAYER - PADDED_WIDTH] = (ch2->getBlockData(off1));
                chLampData[off2 + PADDED_LAYER - PADDED_WIDTH] = ch2->getLampLight(off1);
                chSunlightData[off2 + PADDED_LAYER - PADDED_WIDTH] = ch2->getSunlight(off1);
                chTertiaryData[off2 + PADDED_LAYER - PADDED_WIDTH] = ch2->getTertiaryData(off1);
            }
        }
        ch2->unlock();
        ch1->unlock();
    }
    else{
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            for (x = 1; x < PADDED_WIDTH - 1; x++){
                off1 = (x - 1) + (y - 1)*CHUNK_LAYER;
                off2 = x + y*PADDED_LAYER;
            
                chLampData[off2] = 0;
                chSunlightData[off2] = 0;
                chTertiaryData[off2] = 0;
                chLampData[off2 + PADDED_LAYER - PADDED_WIDTH] = 0;
                chSunlightData[off2 + PADDED_LAYER - PADDED_WIDTH] = 0;
                chTertiaryData[off2 + PADDED_LAYER - PADDED_WIDTH] = 0;
                chData[off2 + PADDED_LAYER - PADDED_WIDTH] = 0;
                chData[off2] = 0;
            }
        }
    }
    //topleft
    ch1 = top->left;
    if (ch1 && ch1->isAccessible){
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
    }

    //topright
    ch1 = top->right;
    if (ch1 && ch1->isAccessible){
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
    }


    //backtop
    ch1 = back->top;
    if (ch1 && ch1->isAccessible){
        ch1->lock();
        for (x = 1; x < PADDED_WIDTH - 1; x++){
            off1 = CHUNK_LAYER - CHUNK_WIDTH + x - 1;
            off2 = PADDED_SIZE - PADDED_LAYER + x;
        
            chData[off2] = (ch1->getBlockData(off1));
            chLampData[off2] = ch1->getLampLight(off1);
            chSunlightData[off2] = ch1->getSunlight(off1);
            chTertiaryData[off2] = ch1->getTertiaryData(off1);
        }
        ch1->unlock();
    }
    

    //fronttop
    ch1 = front->top;
    if (ch1 && ch1->isAccessible){
        ch1->lock();
        for (x = 1; x < PADDED_WIDTH - 1; x++){
            off1 = x - 1;
            off2 = PADDED_SIZE - PADDED_WIDTH + x;

            chData[off2] = (ch1->getBlockData(off1));
            chLampData[off2] = ch1->getLampLight(off1);
            chSunlightData[off2] = ch1->getSunlight(off1);
            chTertiaryData[off2] = ch1->getTertiaryData(off1);
        }
        ch1->unlock();
    }

    //leftback
    ch1 = left->back;
    if (ch1 && ch1->isAccessible){
        ch1->lock();
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            off1 = y*CHUNK_LAYER - 1;
            off2 = y*PADDED_LAYER;

            chData[off2] = (ch1->getBlockData(off1));
            chLampData[off2] = ch1->getLampLight(off1);
            chSunlightData[off2] = ch1->getSunlight(off1);
            chTertiaryData[off2] = ch1->getTertiaryData(off1);
        }
        ch1->unlock();
    }
    
    //rightback
    ch1 = right->back;
    if (ch1 && ch1->isAccessible){
        ch1->lock();
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            off1 = y*CHUNK_LAYER - CHUNK_WIDTH;
            off2 = y*PADDED_LAYER + PADDED_WIDTH - 1;
            
            chData[off2] = (ch1->getBlockData(off1));
            chLampData[off2] = ch1->getLampLight(off1);
            chSunlightData[off2] = ch1->getSunlight(off1);
            chTertiaryData[off2] = ch1->getTertiaryData(off1);
        }
        ch1->unlock();
    }

    //leftfront
    ch1 = left->front;
    if (ch1 && ch1->isAccessible){
        ch1->lock();
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            off1 = (y - 1)*CHUNK_LAYER + CHUNK_WIDTH - 1;
            off2 = (y + 1)*PADDED_LAYER - PADDED_WIDTH;
            
            chData[off2] = (ch1->getBlockData(off1));
            chLampData[off2] = ch1->getLampLight(off1);
            chSunlightData[off2] = ch1->getSunlight(off1);
            chTertiaryData[off2] = ch1->getTertiaryData(off1);
        }
        ch1->unlock();
    }

    //rightfront
    ch1 = right->front;
    if (ch1 && ch1->isAccessible){
        ch1->lock();
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            off1 = (y - 1)*CHUNK_LAYER;
            off2 = (y + 1)*PADDED_LAYER - 1;

            chData[off2] = (ch1->getBlockData(off1));
            chLampData[off2] = ch1->getLampLight(off1);
            chSunlightData[off2] = ch1->getSunlight(off1);
            chTertiaryData[off2] = ch1->getTertiaryData(off1);
        }
        ch1->unlock();
    }
}

void Chunk::addToChunkList(boost::circular_buffer<Chunk*> *chunkListPtr) {
    _chunkListPtr = chunkListPtr;
    chunkListPtr->push_back(this);
}

void Chunk::clearChunkListPtr() {
    _chunkListPtr = nullptr;
}

int Chunk::getRainfall(int xz) const {
    return (int)chunkGridData->heightData[xz].rainfall;
}

int Chunk::getTemperature(int xz) const {
    return (int)chunkGridData->heightData[xz].temperature;
}

void ChunkSlot::clearNeighbors() {
    if (left) {
        if (left->right == this) {
            left->right = nullptr;
            left->numNeighbors--;
        }
        left = nullptr;
    }
    if (right) {
        if (right->left == this) {
            right->left = nullptr;
            right->numNeighbors--;
        }
        right = nullptr;
    }
    if (top) {
        if (top->bottom == this) {
            top->bottom = nullptr;
            top->numNeighbors--;
        }
        top = nullptr;
    }
    if (bottom) {
        if (bottom->top == this) {
            bottom->top = nullptr;
            bottom->numNeighbors--;
        }
        bottom = nullptr;
    }
    if (front) {
        if (front->back == this) {
            front->back = nullptr;
            front->numNeighbors--;
        }
        front = nullptr;
    }
    if (back) {
        if (back->front == this) {
            back->front = nullptr;
            back->numNeighbors--;
        }
        back = nullptr;
    }
    numNeighbors = 0;
}

void ChunkSlot::detectNeighbors(std::unordered_map<i32v3, ChunkSlot*>& chunkSlotMap) {

    std::unordered_map<i32v3, ChunkSlot*>::iterator it;

    i32v3 chPos;
    chPos.x = fastFloor(position.x / (double)CHUNK_WIDTH);
    chPos.y = fastFloor(position.y / (double)CHUNK_WIDTH);
    chPos.z = fastFloor(position.z / (double)CHUNK_WIDTH);

    //left
    if (left == nullptr) {
        it = chunkSlotMap.find(chPos + i32v3(-1, 0, 0));
        if (it != chunkSlotMap.end()) {
            left = it->second;
            left->right = this;
            numNeighbors++;
            left->numNeighbors++;
        }
    }
    //right
    if (right == nullptr) {
        it = chunkSlotMap.find(chPos + i32v3(1, 0, 0));
        if (it != chunkSlotMap.end()) {
            right = it->second;
            right->left = this;
            numNeighbors++;
            right->numNeighbors++;
        }
    }

    //back
    if (back == nullptr) {
        it = chunkSlotMap.find(chPos + i32v3(0, 0, -1));
        if (it != chunkSlotMap.end()) {
            back = it->second;
            back->front = this;
            numNeighbors++;
            back->numNeighbors++;
        }
    }

    //front
    if (front == nullptr) {
        it = chunkSlotMap.find(chPos + i32v3(0, 0, 1));
        if (it != chunkSlotMap.end()) {
            front = it->second;
            front->back = this;
            numNeighbors++;
            front->numNeighbors++;
        }
    }

    //bottom
    if (bottom == nullptr) {
        it = chunkSlotMap.find(chPos + i32v3(0, -1, 0));
        if (it != chunkSlotMap.end()) {
            bottom = it->second;
            bottom->top = this;
            numNeighbors++;
            bottom->numNeighbors++;
        }
    }

    //top
    if (top == nullptr) {
        it = chunkSlotMap.find(chPos + i32v3(0, 1, 0));
        if (it != chunkSlotMap.end()) {
            top = it->second;
            top->bottom = this;
            numNeighbors++;
            top->numNeighbors++;
        }
    }
}

void ChunkSlot::reconnectToNeighbors() {

    if (chunk) {
        chunk->owner = this;
    }
    if (left) {
        left->right = this;
    }
    if (right) {
        right->left = this;
    }
    if (back) {
        back->front = this;
    }
    if (front) {
        front->back = this;
    }
    if (top) {
        top->bottom = this;
    }
    if (bottom) {
        bottom->top = this;
    }
}

double ChunkSlot::getDistance2(const i32v3& pos, const i32v3& cameraPos) {
    double dx = (cameraPos.x <= pos.x) ? pos.x : ((cameraPos.x > pos.x + CHUNK_WIDTH) ? (pos.x + CHUNK_WIDTH) : cameraPos.x);
    double dy = (cameraPos.y <= pos.y) ? pos.y : ((cameraPos.y > pos.y + CHUNK_WIDTH) ? (pos.y + CHUNK_WIDTH) : cameraPos.y);
    double dz = (cameraPos.z <= pos.z) ? pos.z : ((cameraPos.z > pos.z + CHUNK_WIDTH) ? (pos.z + CHUNK_WIDTH) : cameraPos.z);
    dx = dx - cameraPos.x;
    dy = dy - cameraPos.y;
    dz = dz - cameraPos.z;
    //we dont sqrt the distance since sqrt is slow
    return dx*dx + dy*dy + dz*dz;
}