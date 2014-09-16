#include "stdafx.h"
#include "Chunk.h"

#include "BlockData.h"
#include "Errors.h"
#include "Frustum.h"
#include "GameManager.h"
#include "OpenglManager.h"
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
#include "shader.h"
#include "utils.h"
#include "VoxelUtils.h"

GLuint Chunk::vboIndicesID = 0;

vector <MineralData*> Chunk::possibleMinerals;

glm::mat4 MVP;
glm::mat4 GlobalModelMatrix;

//1735
//1500
double surfaceDensity[9][5][5];

void Chunk::init(const glm::ivec3 &pos, int hzI, int hxI, FaceData *fd){

	topBlocked = leftBlocked = rightBlocked = bottomBlocked = frontBlocked = backBlocked = 0;
	loadStatus = 0;
	freeWaiting = 0;
	hasLoadedSunlight = 0;
	isAccessible = 0;
	inLoadThread = 0;
	inGenerateThread = 0;
	inRenderThread = 0;
	inFinishedMeshes = 0;
	inFinishedChunks = 0;
	inSaveThread = 0;
	dirty = 0;
	//THIS MUST COME BEFORE CLEARBUFFERS
	mesh = NULL;
	clearBuffers();
	setupListPtr = NULL;
	updateIndex = -1;
	setupWaitingTime = 0;
	treeTryTicks = 0;
	faceData = *fd;
	hxIndex = hxI;
	hzIndex = hzI;
    position = pos;
	drawIndex = -1;
	numBlocks = -1;
	state = ChunkStates::LOAD;
	left = NULL;
	right = NULL;
	back = NULL;
	top = NULL;
	bottom = NULL;
	front = NULL;
	neighbors = 0;
	distance2 = 999999.0;
	treesToLoad.clear();
	blockUpdateIndex = 0;

	for (int i = 0; i < 8; i++){
		blockUpdateList[i][0].clear();
		blockUpdateList[i][1].clear();
		activeUpdateList[i] = 0;
	}

	spawnerBlocks.clear();
	drawWater = 0;
	occlude = 0;

#ifdef USEARRAYS
    if (!_data) {
        _data = new ui16[CHUNK_SIZE];
        _sunlightData = new ui8[CHUNK_SIZE];
        _lampLightData = new ui8[CHUNK_SIZE];
    }
#endif
}

vector <Chunk*> *dbgst;

void Chunk::clear(bool clearDraw)
{
#ifdef USEARRAYS
    delete[] _data;
    delete[] _sunlightData;
    delete[] _lampLightData;
    _data = nullptr;
    _sunlightData = nullptr;
    _lampLightData = nullptr;
#else
    _dataTree.clear(); 
    _lampLightTree.clear();
    _sunlightTree.clear();
#endif
    state = ChunkStates::LOAD;
    isAccessible = 0;
    left = right = front = back = top = bottom = NULL;
    setupListPtr = NULL;
    updateIndex = -1;
    treeTryTicks = 0;

    vector<ui16>().swap(spawnerBlocks);
    vector<TreeData>().swap(treesToLoad);
    vector<PlantData>().swap(plantsToLoad);
    vector<ui16>().swap(sunRemovalList);
    vector<ui16>().swap(sunExtendList); 

    //TODO: A better solution v v v
    //Hacky way to clear RWQ
/*    ui32 res;
    while (lightFromMain.try_dequeue(res));
    while (lightFromRight.try_dequeue(res));
    while (lightFromLeft.try_dequeue(res));
    while (lightFromFront.try_dequeue(res));
    while (lightFromBack.try_dequeue(res));
    while (lightFromTop.try_dequeue(res));
    while (lightFromBottom.try_dequeue(res)); */

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
        drawIndex = -1;
    }
}

void Chunk::clearBuffers()
{
	if (mesh){
		ChunkMeshData *cmd = new ChunkMeshData(this);
		cmd->chunkMesh = mesh;
		mesh = NULL;
		cmd->debugCode = 1; 
		gameToGl.enqueue(Message(GL_M_CHUNKMESH, cmd));
	}
}

int Chunk::GetPlantType(int x, int z, Biome *biome)
{
    double typer;
    NoiseInfo *nf;
    for (Uint32 i = 0; i < biome->possibleFlora.size(); i++){
        typer = PseudoRand(x + i*(z + 555) + position.x, z - i*(x + 666) + position.z) + 1.0;
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
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == 0){
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
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == 0){
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
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == 0){
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
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == 0){
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
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == 0){
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
            if (getBlock(y*CHUNK_LAYER + z*CHUNK_WIDTH + x).occlude == 0){
                backBlocked = 0;
                y = CHUNK_WIDTH;
                x = CHUNK_WIDTH;
            }
        }
    }
}

void Chunk::SetupMeshData(RenderTask *renderTask)
{
    PreciseTimer timer;
    timer.start();
    int x, y, z, off1, off2;

    Chunk *ch1, *ch2;
    int wc;
    int c = 0;

    i32v3 pos;
    assert(renderTask->chunk != nullptr);

    ui16* wvec = renderTask->wvec;
    ui16* chData = renderTask->chData;
    ui8 *chLightData[2] = { renderTask->chLightData[0], renderTask->chLightData[1] };

    //copy tables
    memcpy(renderTask->biomes, biomes, sizeof(biomes));
    memcpy(renderTask->temperatures, temperatures, sizeof(temperatures));
    memcpy(renderTask->rainfalls, rainfalls, sizeof(rainfalls));
    memcpy(renderTask->depthMap, depthMap, sizeof(depthMap));

    assert(renderTask->chunk != nullptr);

    //Must have all neighbors
    assert(top && left && right && back && front && bottom);
#ifndef USEARRAYS
    int s = 0;
    //block data
    for (int i = 0; i < _dataTree.size(); i++) {
        for (int j = 0; j < _dataTree[i].length; j++) {
            c = _dataTree[i].getStart() + j;
            assert(c < CHUNK_SIZE);
            getPosFromBlockIndex(c, pos);

            wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);
            if (GETBLOCK(chData[wc]).physicsProperty == PhysicsProperties::P_LIQUID) {
                wvec[s++] = wc;
            }
            chData[wc] = _dataTree[i].data;
        }
    }
    renderTask->wSize = s;
    //lamp data
    c = 0;
    for (int i = 0; i < _lampLightTree.size(); i++) {
        for (int j = 0; j < _lampLightTree[i].length; j++) {
            c = _lampLightTree[i].getStart() + j;

            assert(c < CHUNK_SIZE);
            getPosFromBlockIndex(c, pos);
            wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);

            chLightData[0][wc] = _lampLightTree[i].data;
        }
    }
    //sunlight data
    c = 0;
    for (int i = 0; i < _sunlightTree.size(); i++) {
        for (int j = 0; j < _sunlightTree[i].length; j++) {
            c = _sunlightTree[i].getStart() + j;

            assert(c < CHUNK_SIZE);
            getPosFromBlockIndex(c, pos);
            wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);

            chLightData[1][wc] = _sunlightTree[i].data;
        }
    }

    assert(renderTask->chunk != nullptr);
#else

    int s = 0;
    for (y = 0; y < CHUNK_WIDTH; y++){
        for (z = 0; z < CHUNK_WIDTH; z++){
            for (x = 0; x < CHUNK_WIDTH; x++, c++){
                wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                chData[wc] = _data[c];
                if (GETBLOCK(chData[wc]).physicsProperty == PhysicsProperties::P_LIQUID) {
                    wvec[s++] = wc;
                }
                chLightData[0][wc] = _lampLightData[c];
                chLightData[1][wc] = _sunlightData[c];
            }
        }
    }
    renderTask->wSize = s;
#endif
    //top and bottom
    ch1 = bottom;
    ch2 = top;
    if (ch1 && ch2 && ch1->isAccessible && ch2->isAccessible){
        for (z = 1; z < PADDED_WIDTH - 1; z++){
            for (x = 1; x < PADDED_WIDTH - 1; x++){
                off1 = (z - 1)*CHUNK_WIDTH + x - 1;
                off2 = z*PADDED_WIDTH + x;        

                //data
                chData[off2] = (ch1->getBlockData(CHUNK_SIZE - CHUNK_LAYER + off1)); //bottom
                chLightData[0][off2] = ch1->getLampLight(CHUNK_SIZE - CHUNK_LAYER + off1);
                chLightData[1][off2] = ch1->getSunlight(CHUNK_SIZE - CHUNK_LAYER + off1);
                chData[off2 + PADDED_SIZE - PADDED_LAYER] = (ch2->getBlockData(off1)); //top
                chLightData[0][off2 + PADDED_SIZE - PADDED_LAYER] = ch2->getLampLight(off1);
                chLightData[1][off2 + PADDED_SIZE - PADDED_LAYER] = ch2->getSunlight(off1);
            }
        }
    }
    else{
        for (z = 1; z < PADDED_WIDTH - 1; z++){
            for (x = 1; x < PADDED_WIDTH - 1; x++){
                off1 = (z - 1)*CHUNK_WIDTH + x - 1;
                off2 = z*PADDED_WIDTH + x;
            
                chLightData[0][off2] = 0;
                chLightData[1][off2] = 0;
                chLightData[0][off2 + PADDED_SIZE - PADDED_LAYER] = 0;
                chLightData[1][off2 + PADDED_SIZE - PADDED_LAYER] = 0;
                chData[off2 + PADDED_SIZE - PADDED_LAYER] = 0;
                chData[off2] = 0;
            }
        }
    }
    //bottomleft
    ch1 = bottom->left;
    if (ch1 && ch1->isAccessible){
        for (z = 1; z < PADDED_WIDTH - 1; z++){
            off2 = z*PADDED_WIDTH;            

            chData[off2] = (ch1->getBlockData(CHUNK_SIZE - CHUNK_LAYER + off1)); //bottom
            chLightData[0][off2] = ch1->getLampLight(CHUNK_SIZE - CHUNK_LAYER + off1);
            chLightData[1][off2] = ch1->getSunlight(CHUNK_SIZE - CHUNK_LAYER + off1);
        }

        //bottomleftback
        ch2 = ch1->back;
        if (ch2 && ch2->isAccessible){
            off1 = CHUNK_SIZE - 1;
            off2 = 0;    

            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
        //bottomleftfront
        ch2 = ch1->front;
        if (ch2 && ch2->isAccessible){
            off1 = CHUNK_SIZE - CHUNK_LAYER + CHUNK_WIDTH - 1;
            off2 = PADDED_LAYER - PADDED_WIDTH;    

            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }
    else{
        for (z = 1; z < PADDED_WIDTH - 1; z++){
            chData[z*PADDED_WIDTH] = 0;
            chLightData[0][z*PADDED_WIDTH] = 0;
            chLightData[1][z*PADDED_WIDTH] = 0;
        }

        chData[0] = 0;
        chLightData[0][0] = 0;
        chLightData[1][0] = 0;
        chData[PADDED_LAYER - PADDED_WIDTH] = 0;
        chLightData[0][PADDED_LAYER - PADDED_WIDTH] = 0;
        chLightData[1][PADDED_LAYER - PADDED_WIDTH] = 0;
    }

    //bottomright
    ch1 = bottom->right;
    if (ch1 && ch1->isAccessible){
        for (z = 1; z < PADDED_WIDTH - 1; z++){
            off1 = CHUNK_SIZE - CHUNK_LAYER + (z - 1)*CHUNK_WIDTH;
            off2 = z*PADDED_WIDTH + PADDED_WIDTH - 1;        

            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }

        //bottomrightback
        ch2 = ch1->back;
        if (ch2 && ch2->isAccessible){
            off1 = CHUNK_SIZE - CHUNK_WIDTH;
            off2 = PADDED_WIDTH - 1;

            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
        //bottomrightfront
        ch2 = ch1->front;
        if (ch2 && ch2->isAccessible){
            off1 = CHUNK_SIZE - CHUNK_LAYER;
            off2 = PADDED_LAYER - 1;

            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }

    //backbottom
    ch1 = back->bottom;
    if (ch1 && ch1->isAccessible){
        for (x = 1; x < PADDED_WIDTH - 1; x++){
            off1 = CHUNK_SIZE - CHUNK_WIDTH + x - 1;
            off2 = x;

            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }


    //frontbottom
    ch1 = front->bottom;
    if (ch1 && ch1->isAccessible){
        for (x = 1; x < PADDED_WIDTH - 1; x++){
            off1 = CHUNK_SIZE - CHUNK_LAYER + x - 1;
            off2 = PADDED_LAYER - PADDED_WIDTH + x;
        
            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }


    //left and right
    ch1 = left;
    ch2 = right;
    if (ch1 && ch2 && ch1->isAccessible && ch2->isAccessible){
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            for (z = 1; z < PADDED_WIDTH - 1; z++){
                off1 = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
                off2 = z*PADDED_WIDTH + y*PADDED_LAYER;

                chData[off2] = ch1->getBlockData(off1 + CHUNK_WIDTH - 1); //left
                chLightData[0][off2] = ch1->getLampLight(off1 + CHUNK_WIDTH - 1);
                chLightData[1][off2] = ch1->getSunlight(off1 + CHUNK_WIDTH - 1);
                chData[off2 + PADDED_WIDTH - 1] = (ch2->getBlockData(off1));
                chLightData[0][off2 + PADDED_WIDTH - 1] = ch2->getLampLight(off1);
                chLightData[1][off2 + PADDED_WIDTH - 1] = ch2->getSunlight(off1);
            }
        }
    }
    else{
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            for (z = 1; z < PADDED_WIDTH - 1; z++){
                off1 = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
                off2 = z*PADDED_WIDTH + y*PADDED_LAYER;
            
                chLightData[0][off2] = 0;
                chLightData[1][off2] = 0;
                chLightData[0][off2 + PADDED_WIDTH - 1] = 0;
                chLightData[1][off2 + PADDED_WIDTH - 1] = 0;
                chData[off2 + PADDED_WIDTH - 1] = 0;
                chData[off2] = 0;
            }
        }
    }

    //front and back
    ch1 = back;
    ch2 = front;
    if (ch1 && ch2 && ch1->isAccessible && ch2->isAccessible){
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            for (x = 1; x < PADDED_WIDTH - 1; x++){
                off1 = (x - 1) + (y - 1)*CHUNK_LAYER;
                off2 = x + y*PADDED_LAYER;
            
                chData[off2] = ch1->getBlockData(off1 + CHUNK_LAYER - CHUNK_WIDTH);
                chLightData[0][off2] = ch1->getLampLight(off1 + CHUNK_LAYER - CHUNK_WIDTH);
                chLightData[1][off2] = ch1->getSunlight(off1 + CHUNK_LAYER - CHUNK_WIDTH);
                chData[off2 + PADDED_LAYER - PADDED_WIDTH] = (ch2->getBlockData(off1));
                chLightData[0][off2 + PADDED_LAYER - PADDED_WIDTH] = ch2->getLampLight(off1);
                chLightData[1][off2 + PADDED_LAYER - PADDED_WIDTH] = ch2->getSunlight(off1);
            }
        }
    }
    else{
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            for (x = 1; x < PADDED_WIDTH - 1; x++){
                off1 = (x - 1) + (y - 1)*CHUNK_LAYER;
                off2 = x + y*PADDED_LAYER;
            
                chLightData[0][off2] = 0;
                chLightData[1][off2] = 0;
                chLightData[0][off2 + PADDED_LAYER - PADDED_WIDTH] = 0;
                chLightData[1][off2 + PADDED_LAYER - PADDED_WIDTH] = 0;
                chData[off2 + PADDED_LAYER - PADDED_WIDTH] = 0;
                chData[off2] = 0;
            }
        }
    }
    //topleft
    ch1 = top->left;
    if (ch1 && ch1->isAccessible){
        for (z = 1; z < PADDED_WIDTH - 1; z++){
            off1 = z*CHUNK_WIDTH - 1;
            off2 = z*PADDED_WIDTH + PADDED_SIZE - PADDED_LAYER;
            
            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }

        //topleftback
        ch2 = ch1->back;
        if (ch2 && ch2->isAccessible){
            off1 = CHUNK_LAYER - 1;
            off2 = PADDED_SIZE - PADDED_LAYER;
        
            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
        //topleftfront
        ch2 = ch1->front;
        if (ch2 && ch2->isAccessible){
            off1 = CHUNK_WIDTH - 1;
            off2 = PADDED_SIZE - PADDED_WIDTH;
        
            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }

    //topright
    ch1 = top->right;
    if (ch1 && ch1->isAccessible){
        for (z = 1; z < PADDED_WIDTH - 1; z++){
            off1 = (z - 1)*CHUNK_WIDTH;
            off2 = (z + 1)*PADDED_WIDTH - 1 + PADDED_SIZE - PADDED_LAYER;
        
            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }

        //toprightback
        ch2 = ch1->back;
        if (ch2 && ch2->isAccessible){
            off1 = CHUNK_LAYER - CHUNK_WIDTH;
            off2 = PADDED_SIZE - PADDED_LAYER + PADDED_WIDTH - 1;
        
            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
        //toprightfront
        ch2 = ch1->front;
        if (ch2 && ch2->isAccessible){
            off1 = 0;
            off2 = PADDED_SIZE - 1;
        
            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }


    //backtop
    ch1 = back->top;
    if (ch1 && ch1->isAccessible){
        for (x = 1; x < PADDED_WIDTH - 1; x++){
            off1 = CHUNK_LAYER - CHUNK_WIDTH + x - 1;
            off2 = PADDED_SIZE - PADDED_LAYER + x;
        
            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }
    

    //fronttop
    ch1 = front->top;
    if (ch1 && ch1->isAccessible){
        for (x = 1; x < PADDED_WIDTH - 1; x++){
            off1 = x - 1;
            off2 = PADDED_SIZE - PADDED_WIDTH + x;

            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }

    //leftback
    ch1 = left->back;
    if (ch1 && ch1->isAccessible){
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            off1 = y*CHUNK_LAYER - 1;
            off2 = y*PADDED_LAYER;

            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }
    
    //rightback
    ch1 = right->back;
    if (ch1 && ch1->isAccessible){
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            off1 = y*CHUNK_LAYER - CHUNK_WIDTH;
            off2 = y*PADDED_LAYER + PADDED_WIDTH - 1;
            
            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }

    //leftfront
    ch1 = left->front;
    if (ch1 && ch1->isAccessible){
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            off1 = (y - 1)*CHUNK_LAYER + CHUNK_WIDTH - 1;
            off2 = (y + 1)*PADDED_LAYER - PADDED_WIDTH;
            
            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }

    //rightfront
    ch1 = right->front;
    if (ch1 && ch1->isAccessible){
        for (y = 1; y < PADDED_WIDTH - 1; y++){
            off1 = (y - 1)*CHUNK_LAYER;
            off2 = (y + 1)*PADDED_LAYER - 1;

            chData[off2] = (ch1->getBlockData(off1));
            chLightData[0][off2] = ch1->getLampLight(off1);
            chLightData[1][off2] = ch1->getSunlight(off1);
        }
    }
    std::printf("Time: %f ms\n", timer.stop());
}

int Chunk::getRainfall(int xz) const {
    return (int)rainfalls[xz];
}

int Chunk::getTemperature(int xz) const {
    return (int)temperatures[xz];
}