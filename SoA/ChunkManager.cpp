#include "stdafx.h"
#include "ChunkManager.h"

#include <deque>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <ZLIB/zlib.h>

#include "BlockData.h"
#include "CAEngine.h"
#include "Chunk.h"
#include "ChunkUpdater.h"
#include "FileSystem.h"
#include "FloraGenerator.h"
#include "FrameBuffer.h"
#include "Frustum.h"
#include "Mesh.h"
#include "OpenglManager.h"
#include "Options.h"
#include "Particles.h"
#include "PhysicsEngine.h"
#include "Planet.h"
#include "RenderTask.h"
#include "Sound.h"
#include "TaskQueueManager.h"
#include "TerrainGenerator.h"
#include "TerrainPatch.h"
#include "ThreadPool.h"
#include "VRayHelper.h"
#include "VoxelLightEngine.h"
#include "VoxelRay.h"
#include "Vorb.h"
#include "ChunkIOManager.h"
#include "shader.h"

const f32 skyR = 135.0f / 255.0f, skyG = 206.0f / 255.0f, skyB = 250.0f / 255.0f;

const i32 CTERRAIN_PATCH_WIDTH = 5;

//TEXTURE TUTORIAL STUFF

//5846 5061
ChunkManager::ChunkManager() : _isStationary(0) {
    generateOnly = false;
    _mRenderTask = new RenderTask;
    _isHugeShift = 0;
    NoChunkFade = 0;
    _physicsDisabled = 0;
    planet = NULL;
    _maxLoads = 80;
    _poccx = _poccy = _poccz = -1;

    // Clear Out The Chunk Diagnostics
    memset(&_chunkDiagnostics, 0, sizeof(ChunkDiagnostics));

    i32 ci = 0;
    for (i32 i = 0; i < 36; i += 6, ci += 4) {
        starboxIndices[i / 6][i % 6] = ci;
        starboxIndices[i / 6][i % 6 + 1] = ci + 1;
        starboxIndices[i / 6][i % 6 + 2] = ci + 2;
        starboxIndices[i / 6][i % 6 + 3] = ci + 2;
        starboxIndices[i / 6][i % 6 + 4] = ci + 3;
        starboxIndices[i / 6][i % 6 + 5] = ci;
    }

    _maxChunkTicks = 4;

    GlobalModelMatrix = glm::mat4(1.0);
}

ChunkManager::~ChunkManager() {
    deleteAllChunks();
    delete _mRenderTask;
}

//This is hard coded and bad, we need a better method
void ChunkManager::initializeMinerals() {
    //                                           type            sheit  schanc cheit  cchanc   eheit   echanc   mins maxs
    Chunk::possibleMinerals.push_back(new MineralData(TRITANIUM, -800, 0.1f, -2000, 5.0f, -5000000, 3.0f, 3, 30));
    Chunk::possibleMinerals.push_back(new MineralData(URANIUM, -900, 0.1f, -3000, 5.0f, -5000000, 3.0f, 3, 50));
    Chunk::possibleMinerals.push_back(new MineralData(DIAMOND, -150, 1.0f, -1000, 10.0f, -5000000, 9.0f, 1, 6));
    Chunk::possibleMinerals.push_back(new MineralData(RUBY, -40, 1.0f, -500, 15.0f, -5000000, 10.0f, 1, 6));
    Chunk::possibleMinerals.push_back(new MineralData(EMERALD, -35, 1.0f, -500, 15.0f, -5000000, 10.0f, 1, 6));
    Chunk::possibleMinerals.push_back(new MineralData(SAPPHIRE, -30, 1.0f, -500, 15.0f, -5000000, 10.0f, 1, 6));
    Chunk::possibleMinerals.push_back(new MineralData(EMBERNITE, -50, 1.0f, -700, 25.0f, -5000000, 10.0f, 3, 30));
    Chunk::possibleMinerals.push_back(new MineralData(SULFUR, -100, 3.0f, -600, 25.0f, -5000000, 10.0f, 3, 100));
    Chunk::possibleMinerals.push_back(new MineralData(CHROALLON, -120, 1.0f, -500, 15.0f, -5000000, 10.0f, 3, 20));
    Chunk::possibleMinerals.push_back(new MineralData(SEAMORPHITE, 95, 1.0f, -900, 20.0f, -5000000, 10.0f, 3, 50));
    Chunk::possibleMinerals.push_back(new MineralData(THORNMITE, -600, 1.0f, -1200, 10.0f, -5000000, 10.0f, 3, 10));
    Chunk::possibleMinerals.push_back(new MineralData(MORPHIUM, -100, 1.0f, -500, 15.0f, -5000000, 10.0f, 3, 25));
    Chunk::possibleMinerals.push_back(new MineralData(OXYGENIUM, 0, 1.0f, -100, 5.0f, -200, 3.0f, 3, 6));
    Chunk::possibleMinerals.push_back(new MineralData(SUNANITE, -200, 1.0f, -800, 25.0f, -5000000, 10.0f, 3, 25));
    Chunk::possibleMinerals.push_back(new MineralData(CAMONITE, -10, 0.1f, -800, 1.0f, -5000000, 0.4f, 3, 10));
    Chunk::possibleMinerals.push_back(new MineralData(SUBMARIUM, -50, 1.0f, -600, 15.0f, -5000000, 10.0f, 3, 50));
    Chunk::possibleMinerals.push_back(new MineralData(PLATINUM, -100, 1.0f, -900, 20.0f, -5000000, 10.0f, 3, 50));
    Chunk::possibleMinerals.push_back(new MineralData(MALACHITE, -90, 1.0f, -800, 20.0f, -5000000, 10.0f, 3, 30));
    Chunk::possibleMinerals.push_back(new MineralData(MAGNETITE, -60, 1.0f, -700, 20.0f, -5000000, 10.0f, 3, 25));
    Chunk::possibleMinerals.push_back(new MineralData(BAUXITE, -40, 1.0f, -300, 20.0f, -5000000, 10.0f, 3, 60));
    Chunk::possibleMinerals.push_back(new MineralData(LEAD, -40, 1.0f, -600, 20.0f, -5000000, 10.0f, 3, 50));
    Chunk::possibleMinerals.push_back(new MineralData(SILVER, -20, 1.2f, -600, 20.0f, -5000000, 10.0f, 3, 30));
    Chunk::possibleMinerals.push_back(new MineralData(GOLD, -20, 1.0f, -600, 20.0f, -5000000, 10.0f, 3, 30));
    Chunk::possibleMinerals.push_back(new MineralData(IRON, -15, 2.0f, -550, 25.0f, -5000000, 10.0f, 3, 90));
    Chunk::possibleMinerals.push_back(new MineralData(DOLOMITE, -10, 2.0f, -400, 20.0f, -5000000, 10.0f, 3, 200));
    Chunk::possibleMinerals.push_back(new MineralData(TIN, -5, 4.0f, -200, 20.0f, -5000000, 10.0f, 3, 200));
    Chunk::possibleMinerals.push_back(new MineralData(COPPER, -5, 4.0f, -300, 20.0f, -5000000, 10.0f, 3, 200));
    Chunk::possibleMinerals.push_back(new MineralData(COAL, -10, 3.0f, -500, 35.0f, -5000000, 10.0f, 3, 300));
}

void ChunkManager::initialize(const f64v3& gridPosition, int face, ui32 flags) {
    _playerFace = new FaceData(face, 0, 0, 0);
    initializeMinerals();
    GLubyte sbuffer[64][3];
    //sun color map!
    glBindTexture(GL_TEXTURE_2D, GameManager::planet->sunColorMapTexture.ID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, sbuffer);
    for (i32 i = 0; i < 64; i++) {
        sunColor[i][0] = (i32)sbuffer[i][2]; //converts bgr to rgb
        sunColor[i][1] = (i32)sbuffer[i][1];
        sunColor[i][2] = (i32)sbuffer[i][0];
    }

    GameManager::chunkIOManager->beginThread();

    initializeGrid(gridPosition, flags);
}

void ChunkManager::initializeGrid(const f64v3& gpos, ui32 flags) {

    csGridWidth = 1 + (graphicsOptions.voxelRenderDistance / 32) * 2;
    cout << "GRID WIDTH: " << csGridWidth << endl;
    csGridSize = csGridWidth * csGridWidth * csGridWidth;
    _chunkSlotIndexMap.reserve(csGridSize);

    _freeList.set_capacity(csGridSize);
    _setupList.set_capacity(csGridSize);
    _meshList.set_capacity(csGridSize);
    _loadList.set_capacity(csGridSize);
    _generateList.set_capacity(csGridSize);

    _hz = 0;
    _hx = 0;

    calculateCornerPosition(gpos);

    initializeHeightMap();

    if (flags & FLAT_GRASS) {
       /* for (size_t i = 0; i < _heightMap.size(); i++) {
            for (size_t j = 0; j < _heightMap[i].size(); j++) {
                for (i32 k = 0; k < CHUNK_LAYER; k++) {
                    _heightMap[i][j][k].height = 1;
                    _heightMap[i][j][k].rainfall = 128;
                    _heightMap[i][j][k].temperature = 128;
                    _heightMap[i][j][k].flags = 0;
                    _heightMap[i][j][k].biome = planet->allBiomesLookupVector[0];
                    _heightMap[i][j][k].sandDepth = 0;
                    _heightMap[i][j][k].snowDepth = 0;
                    _heightMap[i][j][k].surfaceBlock = DIRTGRASS;
                }
                prepareHeightMap(_heightMap[i][j], 0, 0, CHUNK_WIDTH, CHUNK_WIDTH);
            }
        }*/
    }

    i32 mid = csGridWidth / 2;

    //flatgrass map
    if (flags & SET_Y_TO_SURFACE) {
        if (!(flags & FLAT_GRASS)) {//generate a single height so we know where the player should go
    //        currTerrainGenerator->GenerateHeightMap(_heightMap[mid][mid], (_faceMap[mid][mid]->ipos*CHUNK_WIDTH - planet->radius), (_faceMap[mid][mid]->jpos*CHUNK_WIDTH - planet->radius), CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, 1, 0);
   //         prepareHeightMap(_heightMap[mid][mid], 0, 0, CHUNK_WIDTH, CHUNK_WIDTH);

        }
        i32v2 centerPos;
        centerPos.x = cornerPosition.x + (csGridWidth / 2 + 1) * CHUNK_WIDTH;
        centerPos.y = cornerPosition.z + (csGridWidth / 2 + 1) * CHUNK_WIDTH;

        ChunkGridData* chunkGridData = getChunkGridData(centerPos);
        i32 yPos = chunkGridData->heightData[CHUNK_LAYER / 2].height + chunkGridData->heightData[CHUNK_LAYER / 2].snowDepth + chunkGridData->heightData[CHUNK_LAYER / 2].sandDepth + 15;
        if (yPos < 2) yPos = 2; //keeps player from spawning at the bottom of the ocean

        cornerPosition.y = (i32)floor((yPos - CHUNK_WIDTH * csGridWidth / 2.0) / 32.0) * 32;
    }
}

void ChunkManager::resizeGrid(const f64v3& gpos) {
    //DrawLoadingScreen("Resizing Chunk Grid...", false, glm::vec4(0.0, 0.0, 0.0, 0.3));

    clearAllChunks(1);

  /*  for (i32 y = 0; y < csGridWidth; y++) {
        for (i32 z = 0; z < csGridWidth; z++) {
            for (i32 x = 0; x < csGridWidth; x++) {
                freeChunk(_chunkList[y][z][x]->chunk);
            }
        }
    }*/

    for (size_t i = 0; i < _threadWaitingChunks.size(); i++) { //kill the residual waiting threads too
        recycleChunk(_threadWaitingChunks[i]);
    }
    _threadWaitingChunks.clear();

    vector<ChunkSlot>().swap(_chunkSlots[0]);

    initializeGrid(gpos, 0);
    initializeChunks();
}

void ChunkManager::initializeHeightMap() {
    i32v2 centerPos;
    centerPos.x = cornerPosition.x + (csGridWidth / 2 + 1) * CHUNK_WIDTH;
    centerPos.y = cornerPosition.z + (csGridWidth / 2 + 1) * CHUNK_WIDTH;

    i32 rot = _playerFace->rotation;
    i32 istrt = (cornerPosition.z + planet->radius) / CHUNK_WIDTH + csGridWidth / 2 + 1;
    i32 jstrt = (cornerPosition.x + planet->radius) / CHUNK_WIDTH + csGridWidth / 2 + 1;
    i32 face = _playerFace->face;

    ChunkGridData* newData = new ChunkGridData(face, istrt, jstrt, rot);
    _chunkGridDataMap[centerPos] = newData;
}

void ChunkManager::initializeChunks() {
    i32 c = 0;
    int z = csGridWidth / 2 + 1;
    int x = csGridWidth / 2 + 1;
    int y = csGridWidth / 2 + 1;

    _chunkSlots[0].reserve(125000);
    i32v2 gridPos(x * CHUNK_WIDTH + cornerPosition.x, z * CHUNK_WIDTH + cornerPosition.z);
    ChunkGridData* chunkGridData = getChunkGridData(gridPos);
    if (chunkGridData == nullptr) {
        pError("NULL grid data at initializeChunks()");
    }
    _chunkSlots[0].emplace_back(cornerPosition + glm::ivec3(x, y, z) * CHUNK_WIDTH, nullptr, z, x, chunkGridData);

    //we dont sqrt the distance since sqrt is slow
    _chunkSlots[0][0].distance2 = 1;

    Chunk* chunk = produceChunk();
    chunk->init(_chunkSlots[0][0].position, &_chunkSlots[0][0]);
    chunk->distance2 = _chunkSlots[0][0].distance2;
    chunk->chunkGridData = chunkGridData;
    _chunkSlots[0][0].chunk = chunk;
    _chunkSlots[0][0].vecIndex = 0;
    addToLoadList(chunk);

    _chunkSlotIndexMap[chunk->chunkPosition] = 0;
}

void ChunkManager::clearAllChunks(bool clearDrawing) {
    threadPool.clearJobs();
    while (!(threadPool.isFinished()));

    taskQueueManager.finishedChunks.clear(); //we know they are finished so just clear it
    for (size_t i = 0; i < taskQueueManager.finishedChunkMeshes.size(); i++) delete taskQueueManager.finishedChunkMeshes[i];
    taskQueueManager.finishedChunkMeshes.clear(); //we know they are finished so just clear it
    GameManager::chunkIOManager->clear();
  
    _setupList.clear();
    _generateList.clear();
    _meshList.clear();
    _loadList.clear();
    queue <ChunkMeshData *>().swap(_finishedChunkMeshes); //clear it

    for (size_t i = 0; i < _threadWaitingChunks.size(); i++) {
        recycleChunk(_threadWaitingChunks[i]);
        _threadWaitingChunks[i]->inRenderThread = 0;
        _threadWaitingChunks[i]->inLoadThread = 0;
        _threadWaitingChunks[i]->inGenerateThread = 0;
        _threadWaitingChunks[i]->inSaveThread = 0;
        _threadWaitingChunks[i]->inFinishedChunks = 0;
        _threadWaitingChunks[i]->inFinishedMeshes = 0;
    }
    _threadWaitingChunks.clear();

    if (clearDrawing) {
        for (i32 i = 0; i < _chunkSlots[0].size(); i++) {
            if (_chunkSlots[0][i].chunk) _chunkSlots[0][i].chunk->clear();
        }
    } else {
        for (i32 i = 0; i < csGridSize; i++) {
            if (_chunkSlots[0][i].chunk) _chunkSlots[0][i].chunk->clear(0);
        }
    }
}

void ChunkManager::clearAll() {
    clearAllChunks(true);
    threadPool.close();

    for (size_t i = 0; i < Chunk::possibleMinerals.size(); i++) {
        delete Chunk::possibleMinerals[i];
    }
    Chunk::possibleMinerals.clear();

    for (auto it = _chunkGridDataMap.begin(); it != _chunkGridDataMap.end(); it++) {
        delete it->second;
    }
    _chunkGridDataMap.clear();

    for (i32 i = 0; i < _chunkSlots[0].size(); i++) {
        freeChunk(_chunkSlots[0][i].chunk);
    }

    GameManager::physicsEngine->clearAll();

    for (size_t i = 0; i < _threadWaitingChunks.size(); i++) { //kill the residual waiting threads too
        recycleChunk(_threadWaitingChunks[i]);
    }
    _threadWaitingChunks.clear();

    deleteAllChunks();

    vector<ChunkSlot>().swap(_chunkSlots[0]);
}

void ChunkManager::relocateChunks(const f64v3& gpos) {
    //save all chunks
    vector<Chunk*> toSave;
    Chunk* chunk;
    for (i32 i = 0; i < _chunkSlots[0].size(); i++) {
        chunk = _chunkSlots[0][i].chunk;
        if (chunk) {
            if (chunk->dirty && chunk->_state > ChunkStates::TREES) {
                toSave.push_back(chunk);
            }
        }
    }
    GameManager::chunkIOManager->addToSaveList(toSave);

    calculateCornerPosition(gpos);

    threadPool.clearJobs();
    while (!(threadPool.isFinished()));
    taskQueueManager.finishedChunks.clear(); //we know they are finished so just clear it
    for (size_t i = 0; i < taskQueueManager.finishedChunkMeshes.size(); i++) delete taskQueueManager.finishedChunkMeshes[i];
    taskQueueManager.finishedChunkMeshes.clear(); //we know they are finished so just clear it
    GameManager::chunkIOManager->clear();
    _setupList.clear();
    _generateList.clear();
    _meshList.clear();
    _loadList.clear();

    _finishedChunkMeshes.swap(queue <ChunkMeshData *>()); //clear it

    for (size_t i = 0; i < _threadWaitingChunks.size(); i++) {
        recycleChunk(_threadWaitingChunks[i]);
        _threadWaitingChunks[i]->inRenderThread = 0;
        _threadWaitingChunks[i]->inLoadThread = 0;
        _threadWaitingChunks[i]->inGenerateThread = 0;
        _threadWaitingChunks[i]->inSaveThread = 0;
        _threadWaitingChunks[i]->inFinishedChunks = 0;
        _threadWaitingChunks[i]->inFinishedMeshes = 0;
    }
    _threadWaitingChunks.clear();

    _hx = 0;
    _hz = 0;

    i32 istrt = (cornerPosition.z + planet->radius) / CHUNK_WIDTH;
    i32 jstrt = (cornerPosition.x + planet->radius) / CHUNK_WIDTH;

    while (istrt < 0) {
        istrt += (planet->radius * 2) / CHUNK_WIDTH;
    }

    while (istrt >= (planet->radius * 2) / CHUNK_WIDTH) {
        istrt -= (planet->radius * 2) / CHUNK_WIDTH;
    }

    while (jstrt < 0) {
        jstrt += (planet->radius * 2) / CHUNK_WIDTH;
    }

    while (jstrt >= (planet->radius * 2) / CHUNK_WIDTH) {
        jstrt -= (planet->radius * 2) / CHUNK_WIDTH;
    }

    for (auto it = _chunkGridDataMap.begin(); it != _chunkGridDataMap.end(); it++) {
        delete it->second;
    }
    _chunkGridDataMap.clear();

    for (int i = 0; i < _chunkSlots[0].size(); i++) {
        chunk = _chunkSlots[0][i].chunk;
        chunk->updateIndex = -1;
        chunk->drawIndex = -1;
        chunk->setupListPtr = NULL; //we already cleared the setup lists
        chunk->inLoadThread = chunk->inSaveThread = 0;
        freeChunk(chunk);
    }

    initializeChunks();
}

void ChunkManager::clearChunkData() {

    Chunk* chunk;
    for (i32 y = 0; y < csGridWidth; y++) {
        for (i32 z = 0; z < csGridWidth; z++) {
            for (i32 x = 0; x < csGridWidth; x++) {
                chunk = _chunkSlots[0][y * csGridWidth * csGridWidth + z * csGridWidth + x].chunk;
                if (chunk) {
                    clearChunk(chunk);
                    recycleChunk(chunk);
                }
            }
        }
    }
}

i32 pshift = 0;

i32 ticksArrayIndex = 0;
i32 ticksArray[10];

void ChunkManager::update(const f64v3& position, const f64v3& viewDir) {
    globalMultiplePreciseTimer.setDesiredSamples(10);
    globalMultiplePreciseTimer.start("Update");
   
    static f64v3 oldPosition = position;

    float dPos = glm::length(position - oldPosition);
    oldPosition = position;

    static i32 k = 0, cl = 0;
    static i32 setupStart = 10000;

    sonarDt += 0.003f*physSpeedFactor;
    if (sonarDt > 1.0f) sonarDt = 0.0f;

    i32 shiftType = 0; //1 = only sort chunks, 2 = chunks and lods
    glm::ivec3 playerVoxelPosition;
    playerVoxelPosition.x = fastFloor(position.x);
    playerVoxelPosition.y = fastFloor(position.y);
    playerVoxelPosition.z = fastFloor(position.z);

    shiftType = 0;

    const double maxDPos = 64;
    if (dPos >= maxDPos) {
        _isHugeShift = true;
        shiftType = 1;
    } else if (_isHugeShift) {
        _isHugeShift = false;
        relocateChunks(position);
    }
    
    const i32 CENTER_OFFSET = (csGridWidth / 2) * CHUNK_WIDTH + CHUNK_WIDTH / 2;
    const i32 DISTANCE_THRESHOLD = CHUNK_WIDTH;

    //lock to prevent collision race condition with player in opengl thread
    if (!_isStationary && dPos && dPos < maxDPos) {
        openglManager.collisionLock.lock();
        if (playerVoxelPosition.y > cornerPosition.y + CENTER_OFFSET + DISTANCE_THRESHOLD) {
            while (playerVoxelPosition.y > cornerPosition.y + CENTER_OFFSET + DISTANCE_THRESHOLD) {
                shiftY(1);
                shiftType = 1;
            }
        } else if (playerVoxelPosition.y < cornerPosition.y + CENTER_OFFSET - DISTANCE_THRESHOLD) {
            while (playerVoxelPosition.y < cornerPosition.y + CENTER_OFFSET - DISTANCE_THRESHOLD) {
                shiftY(-1);
                shiftType = 1;
            }
        }
        if (playerVoxelPosition.x > cornerPosition.x + CENTER_OFFSET + DISTANCE_THRESHOLD) {
            while (playerVoxelPosition.x > cornerPosition.x + CENTER_OFFSET + DISTANCE_THRESHOLD) {
                shiftX(1);
                shiftType = 2;
            }
        } else if (playerVoxelPosition.x < cornerPosition.x + CENTER_OFFSET - DISTANCE_THRESHOLD) {
            while (playerVoxelPosition.x < cornerPosition.x + CENTER_OFFSET - DISTANCE_THRESHOLD) {
                shiftX(-1);
                shiftType = 2;
            }
        }
        if (playerVoxelPosition.z > cornerPosition.z + CENTER_OFFSET + DISTANCE_THRESHOLD) {
            while (playerVoxelPosition.z > cornerPosition.z + CENTER_OFFSET + DISTANCE_THRESHOLD) {
                shiftZ(1);
                shiftType = 2;
            }
        } else if (playerVoxelPosition.z < cornerPosition.z + CENTER_OFFSET - DISTANCE_THRESHOLD) {
            while (playerVoxelPosition.z < cornerPosition.z + CENTER_OFFSET - DISTANCE_THRESHOLD) {
                shiftZ(-1);
                shiftType = 2;
            }
        }
        openglManager.collisionLock.unlock();
    }
    globalMultiplePreciseTimer.start("Update Chunks");
    Chunk::modifyLock.lock();
    if (_isStationary) {
        updateChunks(f64v3(cornerPosition) + f64v3(CENTER_OFFSET));
    } else {
        updateChunks(position);
    }
    Chunk::modifyLock.unlock();
    globalMultiplePreciseTimer.start("Update Load List");
    updateLoadList(_maxChunkTicks);
  //  globalMultiplePreciseTimer.start("CAEngine Update");
  //  GameManager::caEngine->update(*this);

    globalMultiplePreciseTimer.start("Loaded Chunks");
    updateLoadedChunks();

    globalMultiplePreciseTimer.start("Sort");
  //  cout << "BEGIN SORT\n";
 //   fflush(stdout);
    if (!shiftType && pshift) {

        recursiveSortSetupList(_setupList, 0, _setupList.size(), 2);
        recursiveSortSetupList(_meshList, 0, _meshList.size(), 2);
        recursiveSortChunks(_loadList, 0, _loadList.size(), 2);
    } else if (!shiftType) {
        if (k >= 8 || (k >= 4 && physSpeedFactor >= 2.0)) {
            recursiveSortSetupList(_setupList, 0, _setupList.size(), 2);
            recursiveSortSetupList(_meshList, 0, _meshList.size(), 2);
            recursiveSortChunks(_loadList, 0, _loadList.size(), 2);
            k = 0;
        }
    }
   // cout << "END SORT\n";
   // fflush(stdout);
    globalMultiplePreciseTimer.start("Mesh List");
    updateMeshList(_maxChunkTicks, position);
    globalMultiplePreciseTimer.start("Generate List");
    updateGenerateList(_maxChunkTicks);
    globalMultiplePreciseTimer.start("Setup List");
    updateSetupList(_maxChunkTicks);

    //This doesnt function correctly
    //caveOcclusion(position);
  //  globalMultiplePreciseTimer.start("Physics Engine");
    //Chunk::modifyLock.lock();
   // GameManager::physicsEngine->update(viewDir);
    //Chunk::modifyLock.unlock();

    k++;
    cl++;
    pshift = shiftType;
    globalMultiplePreciseTimer.start("Thread Waiting");
    Chunk* ch;
    for (size_t i = 0; i < _threadWaitingChunks.size();) {
        ch = _threadWaitingChunks[i];
        if (ch->inSaveThread == 0 && ch->inLoadThread == 0 && ch->inRenderThread == 0 && ch->inGenerateThread == 0) {
            freeChunk(_threadWaitingChunks[i]);
            _threadWaitingChunks[i] = _threadWaitingChunks.back();
            _threadWaitingChunks.pop_back();
        } else {
            i++;
        }
    }
    globalMultiplePreciseTimer.start("Finished Meshes");
    uploadFinishedMeshes();
    //change the parameter to true to print out the timingsm
    globalMultiplePreciseTimer.end(true);
}

//traverses the back of the load list, popping of entries and giving them to threads to be loaded
void ChunkManager::updateLoadList(ui32 maxTicks) {

    Chunk* chunk;
    vector<Chunk* > chunksToLoad;
    ChunkGridData* chunkGridData;

    ui32 sticks = SDL_GetTicks();
    for (i32 i = _loadList.size() - 1; i >= 0; i--) {
        chunk = _loadList[i];
        _loadList.pop_back();
        chunk->updateIndex = -1;
        chunk->setupListPtr = NULL;
     
        chunk->isAccessible = 0;

        chunksToLoad.push_back(chunk);

        if (SDL_GetTicks() - sticks >= maxTicks) {
            break;
        }
    }

    if (chunksToLoad.size()) GameManager::chunkIOManager->addToLoadList(chunksToLoad);
    chunksToLoad.clear();
}

void ChunkManager::uploadFinishedMeshes() {
    Chunk* chunk;
    ChunkMeshData *cmd;

    taskQueueManager.frLock.lock();
    for (size_t i = 0; i < taskQueueManager.finishedChunkMeshes.size(); i++) {
        _finishedChunkMeshes.push(taskQueueManager.finishedChunkMeshes[i]);
        assert(taskQueueManager.finishedChunkMeshes[i]->chunk != nullptr);
        taskQueueManager.finishedChunkMeshes[i]->chunk->inFinishedMeshes = 0;
    }
    taskQueueManager.finishedChunkMeshes.clear();
    taskQueueManager.frLock.unlock();

    //use the temp vector so that the threads dont have to wait.
    while (_finishedChunkMeshes.size()) {
        cmd = _finishedChunkMeshes.front();
        _finishedChunkMeshes.pop();
        chunk = cmd->chunk;
        chunk->inRenderThread = 0;
        //remove the chunk if it is waiting to be freed
        if (chunk->freeWaiting) {
            cmd->chunkMesh = chunk->mesh;
            chunk->mesh = NULL;
            if (cmd->chunkMesh != NULL) {
                cmd->debugCode = 2;
                gameToGl.enqueue(Message(GL_M_CHUNKMESH, cmd));
            }
            continue;
        }
        
        //set add the chunk mesh to the message
        cmd->chunkMesh = chunk->mesh;

        //Check if we need to allocate a new chunk mesh or orphan the current chunk mesh so that it can be freed in openglManager
        switch (cmd->type) {
            case MeshJobType::DEFAULT:
                if (cmd->waterVertices.empty() && cmd->transVertices.empty() && cmd->vertices.empty() && cmd->cutoutVertices.empty()) {
                    chunk->mesh = nullptr;
                } else if (chunk->mesh == nullptr) {
                    chunk->mesh = new ChunkMesh(chunk);
                    cmd->chunkMesh = chunk->mesh;
                }
                break;
            case MeshJobType::LIQUID:
                if (cmd->waterVertices.empty() && (chunk->mesh == nullptr || (chunk->mesh->vboID == 0 && chunk->mesh->cutoutVboID == 0 && chunk->mesh->transVboID == 0))) {
                    chunk->mesh = nullptr;
                } else if (chunk->mesh == nullptr) {
                    chunk->mesh = new ChunkMesh(chunk);
                    cmd->chunkMesh = chunk->mesh;
                }
                break;
        }

        //if the chunk has a mesh, send it
        if (cmd->chunkMesh) {
            cmd->debugCode = 3;
            gameToGl.enqueue(Message(GL_M_CHUNKMESH, cmd));
        }

        if (chunk->setupListPtr == nullptr) chunk->_state = ChunkStates::DRAW;
    }
}

//add the loaded chunks to the setup list
void ChunkManager::updateLoadedChunks() {
    taskQueueManager.fcLock.lock();
    Chunk* ch;
    for (size_t i = 0; i < taskQueueManager.finishedChunks.size(); i++) {
        ch = taskQueueManager.finishedChunks[i];
        ch->inFinishedChunks = false;
        ch->inGenerateThread = false;
        ch->isAccessible = true;

        if (!(ch->freeWaiting)) {
            setupNeighbors(ch);

            //check to see if the top chunk has light that should end up in this chunk
            VoxelLightEngine::checkTopForSunlight(ch);

            if (ch->treesToLoad.size() || ch->plantsToLoad.size()) {
                ch->_state = ChunkStates::TREES;
                addToSetupList(ch);
            } else {
                ch->_state = ChunkStates::MESH;
                addToMeshList(ch);
            }
        }
    }
    taskQueueManager.finishedChunks.clear();
    taskQueueManager.fcLock.unlock();

    //IO load chunks
    GameManager::chunkIOManager->flcLock.lock();
    for (size_t i = 0; i < GameManager::chunkIOManager->finishedLoadChunks.size(); i++) {

        ch = GameManager::chunkIOManager->finishedLoadChunks[i];
        ch->inLoadThread = 0;

        if (!(ch->freeWaiting)) {
            if (ch->loadStatus == 999) { //retry loading it. probably a mistake
                ch->loadStatus = 0;
                ch->isAccessible = false;
                addToLoadList(ch);
            } else if (ch->loadStatus == 1) { //it is not saved. Generate!
                ch->loadStatus == 0;
                ch->isAccessible = false;
                addToGenerateList(ch);
            } else {
                setupNeighbors(ch);
                ch->_state = ChunkStates::MESH;
                addToMeshList(ch);
                ch->dirty = false;
                ch->isAccessible = true;
            }
        }
    }
    GameManager::chunkIOManager->finishedLoadChunks.clear();
    GameManager::chunkIOManager->flcLock.unlock();
}

i32 ChunkManager::updateSetupList(ui32 maxTicks) {
    Chunk* chunk;
    ChunkStates state;
    i32 i;
    f64v3 cpos;

    ui32 startTicks = SDL_GetTicks();

    for (i = _setupList.size() - 1; i >= 0; i--) {
        //limit the time
        chunk = _setupList[i];
        chunk->setupWaitingTime = 0;
        state = chunk->_state;

        switch (state) {
        case ChunkStates::TREES:
            if (chunk->numNeighbors == 6) {
                if (chunk->treeTryTicks == 0) { //keep us from continuing to try a tree when it wont ever load
                    if (FloraGenerator::generateFlora(chunk)) {
                        chunk->_state = ChunkStates::MESH;

                        _setupList[i] = _setupList.back();
                        _setupList[i]->updateIndex = i;

                        chunk->updateIndex = -1;
                        chunk->setupListPtr = NULL;
                        _setupList.pop_back();

                        addToMeshList(chunk);

                    } else {
                        chunk->treeTryTicks = 1;
                    }
                } else {
                    chunk->treeTryTicks++;
                    if (chunk->treeTryTicks >= 15) { //pause for 15 frames, or roughly 0.25 second
                        chunk->treeTryTicks = 0;
                    }
                }
            }
            break;
        default: // chunks that should not be here should be removed
            cout << "ERROR: Chunk with state " << (int)state << " in setup list.\n";
            _setupList[i] = _setupList.back();
            _setupList[i]->updateIndex = i;
            chunk->updateIndex = -1;
            chunk->setupListPtr = NULL;
            _setupList.pop_back();
            break;
        }

        if (SDL_GetTicks() - startTicks > maxTicks) break;
    }

    return i;
}

i32 ChunkManager::updateMeshList(ui32 maxTicks, const f64v3& position) {

    ui32 startTicks = SDL_GetTicks();
    ChunkStates state;
    Chunk* chunk;

    for (i32 i = _meshList.size() - 1; i >= 0; i--) {
        state = _meshList[i]->_state;
        chunk = _meshList[i];

        if (chunk->numNeighbors == 6 && SphereInFrustum((float)(chunk->gridPosition.x + CHUNK_WIDTH / 2 - position.x), (float)(chunk->gridPosition.y + CHUNK_WIDTH / 2 - position.y), (float)(chunk->gridPosition.z + CHUNK_WIDTH / 2 - position.z), 28.0f, gridFrustum)) {

          
            VoxelLightEngine::calculateLight(chunk);
            
            if (chunk->numBlocks < 0) {
                cout << "CHUNK NUM < 0 == " << chunk->numBlocks << endl;
            }
            //TODO: BEN, Need to make sure chunk->num is always correct
            if (chunk->numBlocks) { 

                chunk->occlude = 0;

                if (chunk->numNeighbors == 6 && chunk->inRenderThread == 0) {
                    if (chunk->_state == ChunkStates::MESH) {
                        if (!threadPool.addRenderJob(chunk, MeshJobType::DEFAULT)) break; //if the queue is full, well try again next frame
                    } else {
                        if (!threadPool.addRenderJob(chunk, MeshJobType::LIQUID)) break;
                    }
                    _meshList[i] = _meshList.back();
                    _meshList[i]->updateIndex = i;

                    chunk->updateIndex = -1;
                    chunk->setupListPtr = NULL;
                    _meshList.pop_back();
                }
            } else {
                chunk->clearBuffers();
                _meshList[i] = _meshList.back();
                _meshList[i]->updateIndex = i;

                chunk->updateIndex = -1;
                chunk->setupListPtr = NULL;
                _meshList.pop_back();

                chunk->_state = ChunkStates::INACTIVE;
            }
        }


        if (SDL_GetTicks() - startTicks > maxTicks) break;
    }
    return 0;
}

i32 ChunkManager::updateGenerateList(ui32 maxTicks) {

    ui32 startTicks = SDL_GetTicks();
    i32 state;
    Chunk* chunk;
    ChunkGridData* chunkGridData;
    i32 startX, startZ;
    i32 ipos, jpos, rpos;
    i32 idir, jdir;
    i32 rot, face;

    while (_generateList.size()) {
        chunk = _generateList.front();
        chunk->updateIndex = -1;
        chunk->setupListPtr = nullptr;

        _generateList.pop_front();

        chunkGridData = chunk->chunkGridData;

        //If the heightmap has not been generated, generate it.
        if (chunkGridData->heightData[0].height == UNLOADED_HEIGHT) {

            //set up the directions
            rot = chunkGridData->faceData.rotation;
            face = chunkGridData->faceData.face;
            ipos = FaceCoords[face][rot][0];
            jpos = FaceCoords[face][rot][1];
            rpos = FaceCoords[face][rot][2];
            idir = FaceSigns[face][rot][0];
            jdir = FaceSigns[face][rot][1];

            currTerrainGenerator->SetLODFace(ipos, jpos, rpos, FaceRadialSign[face] * planet->radius, idir, jdir, 1.0);

            currTerrainGenerator->GenerateHeightMap(chunkGridData->heightData, (chunkGridData->faceData.ipos*CHUNK_WIDTH - planet->radius), (chunkGridData->faceData.jpos*CHUNK_WIDTH - planet->radius), CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, 1, 0);
            prepareHeightMap(chunkGridData->heightData);
        }

        chunk->isAccessible = 0;

        threadPool.addLoadJob(chunk, new LoadData(chunk->chunkGridData->heightData, cornerPosition.x, cornerPosition.z, currTerrainGenerator));

        if (SDL_GetTicks() - startTicks > maxTicks) break;
    }
    return 0;
}

void ChunkManager::setupNeighbors(Chunk* chunk) {
    i32v3 chPos;
    chPos.x = fastFloor(chunk->gridPosition.x / (float)CHUNK_WIDTH);
    chPos.y = fastFloor(chunk->gridPosition.y / (float)CHUNK_WIDTH);
    chPos.z = fastFloor(chunk->gridPosition.z / (float)CHUNK_WIDTH);
    ChunkSlot* neighborSlot;
    ChunkSlot* owner = chunk->owner;
    vector <ChunkSlot>& chunkSlots = _chunkSlots[0];
    //left
    if (owner->left != -1) {
        neighborSlot = &chunkSlots[owner->left];
        if (!(chunk->left) && neighborSlot->chunk->isAccessible) {
            chunk->left = neighborSlot->chunk;
            chunk->numNeighbors++;
            chunk->left->right = chunk;
            chunk->left->numNeighbors++;
        }
    }
    
    //right
    if (owner->right != -1) {
        neighborSlot = &chunkSlots[owner->right];
        if (!(chunk->right) && neighborSlot->chunk->isAccessible) {
            chunk->right = neighborSlot->chunk;
            chunk->numNeighbors++;
            chunk->right->left = chunk;
            chunk->right->numNeighbors++;
        }
    }
    
    //back
    if (owner->back != -1) {
        neighborSlot = &chunkSlots[owner->back];
        if (!(chunk->back) && neighborSlot->chunk->isAccessible) {
            chunk->back = neighborSlot->chunk;
            chunk->numNeighbors++;
            chunk->back->front = chunk;
            chunk->back->numNeighbors++;
        }
    }
    
    //front
    if (owner->front != -1) {
        neighborSlot = &chunkSlots[owner->front];
        if (!(chunk->front) && neighborSlot->chunk->isAccessible) {
            chunk->front = neighborSlot->chunk;
            chunk->numNeighbors++;
            chunk->front->back = chunk;
            chunk->front->numNeighbors++;
        }
    }
    
    //bottom
    if (owner->bottom != -1) {
        neighborSlot = &chunkSlots[owner->bottom];
        if (!(chunk->bottom) && neighborSlot->chunk->isAccessible) {
            chunk->bottom = neighborSlot->chunk;
            chunk->numNeighbors++;
            chunk->bottom->top = chunk;
            chunk->bottom->numNeighbors++;
        }
    }
    
    //top
    if (owner->top != -1) {
        neighborSlot = &chunkSlots[owner->top];
        if (!(chunk->top) && neighborSlot->chunk->isAccessible) {
            chunk->top = neighborSlot->chunk;
            chunk->numNeighbors++;
            chunk->top->bottom = chunk;
            chunk->top->numNeighbors++;
        }
    }
}

void ChunkManager::drawChunkLines(glm::mat4 &VP, const f64v3& position) {
    // Element pattern
    const ui32 elementBuffer[24] = { 0, 1, 0, 2, 1, 3, 2, 3, 4, 5, 4, 6, 5, 7, 6, 7, 0, 4, 1, 5, 2, 6, 3, 7 };
    // Shader that is lazily initialized
    static GLProgram* chunkLineProgram = nullptr;
    // The mesh that is built from the chunks
    vcore::Mesh mesh;
    mesh.init(vcore::PrimitiveType::LINES, true);
    // Reserve the number of vertices and indices we think we will need
    mesh.reserve(_chunkSlots[0].size() * 8, _chunkSlots[0].size() * 24);
    // Build the mesh
    Chunk* chunk;
    ColorRGBA8 color;
    // Used to build each grid
    vector <vcore::MeshVertex> vertices(8);
    vector <ui32> indices(24);
    int numVertices = 0;

    f32v3 posOffset;

    for (i32 i = 0; i < _chunkSlots[0].size(); i++) {
        chunk = _chunkSlots[0][i].chunk;
        posOffset = f32v3(f64v3(chunk->gridPosition) - position);

        if (chunk && ((chunk->mesh && chunk->mesh->inFrustum) || SphereInFrustum((float)(posOffset.x + CHUNK_WIDTH / 2), (float)(posOffset.y + CHUNK_WIDTH / 2), (float)(posOffset.z + CHUNK_WIDTH / 2), 28.0f, gridFrustum))) {
            

            switch (chunk->_state) {
            case ChunkStates::GENERATE:
                color = ColorRGBA8(255, 0, 255, 255);
                break;
            case ChunkStates::LOAD:
                color = ColorRGBA8(255, 255, 255, 255);
                break;
            case ChunkStates::LIGHT:
                color = ColorRGBA8(255, 255, 0, 255);
                break;
            case ChunkStates::TREES:
                color = ColorRGBA8(0, 128, 0, 255);
                break;
            case ChunkStates::DRAW:
                color = ColorRGBA8(0, 0, 255, 255);
                break;
            case ChunkStates::MESH:
                color = ColorRGBA8(0, 255, 0, 255);
                break;
            case ChunkStates::WATERMESH:
                color = ColorRGBA8(0, 255, 255, 255);
                break;
            default:
                color = ColorRGBA8(0, 0, 0, 255);
                break;
            }
            for (int i = 0; i < 8; i++) {
                vertices[i].color = color;
                vertices[i].uv = f32v2(0.0f, 0.0f);
            }
            // Build the indices
            for (int i = 0; i < 24; i++) {
                indices[i] = numVertices + elementBuffer[i];
            }
            numVertices += 8;
            if (chunk->_state != ChunkStates::INACTIVE) {
                // Build the vertices
                const float gmin = 0.00001;
                const float gmax = 31.9999;
                vertices[0].position = f32v3(gmin, gmin, gmin) + posOffset;
                vertices[1].position = f32v3(gmax, gmin, gmin) + posOffset;
                vertices[2].position = f32v3(gmin, gmin, gmax) + posOffset;
                vertices[3].position = f32v3(gmax, gmin, gmax) + posOffset;
                vertices[4].position = f32v3(gmin, gmax, gmin) + posOffset;
                vertices[5].position = f32v3(gmax, gmax, gmin) + posOffset;
                vertices[6].position = f32v3(gmin, gmax, gmax) + posOffset;
                vertices[7].position = f32v3(gmax, gmax, gmax) + posOffset;
            }
            mesh.addVertices(vertices, indices);
        }
    }
    // Check if a non-empty mesh was built
    if (numVertices != 0) {
        // Upload the data
        mesh.uploadAndClearLocal();
        // Lazily initialize shader
        if (chunkLineProgram == nullptr) {
            chunkLineProgram = new GLProgram(true);
            chunkLineProgram->addShader(ShaderType::VERTEX, vcore::Mesh::defaultVertexShaderSource);
            chunkLineProgram->addShader(ShaderType::FRAGMENT, vcore::Mesh::defaultFragmentShaderSource);
            chunkLineProgram->setAttributes(vcore::Mesh::defaultShaderAttributes);
            chunkLineProgram->link();
            chunkLineProgram->initUniforms();
        }
        // Bind the program
        chunkLineProgram->use();
        // Set Matrix
        glUniformMatrix4fv(chunkLineProgram->getUniform("MVP"), 1, GL_FALSE, &(VP[0][0]));
        // Set Texture
        glUniform1i(chunkLineProgram->getUniform("tex"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, BlankTextureID.ID);
        // Draw the grid
        mesh.draw();
        // Unuse the program
        chunkLineProgram->unuse();
    }
}

void ChunkManager::freeChunk(Chunk* chunk) {
    if (chunk) {
        if (!generateOnly && chunk->dirty && chunk->_state > ChunkStates::TREES) {
            GameManager::chunkIOManager->addToSaveList(chunk);
        }
        if (chunk->inSaveThread || chunk->inRenderThread || chunk->inLoadThread || chunk->inGenerateThread) {
            clearChunkFromLists(chunk);
            chunk->freeWaiting = true;
            _threadWaitingChunks.push_back(chunk);
        } else {
            clearChunk(chunk);
            recycleChunk(chunk);
        }
    }
}

void ChunkManager::addToSetupList(Chunk* chunk) {
    chunk->setupListPtr = &_setupList;
    chunk->updateIndex = _setupList.size();
    _setupList.push_back(chunk);
}

void ChunkManager::addToLoadList(Chunk* chunk) {
    chunk->_state = ChunkStates::LOAD;
    chunk->setupListPtr = &_loadList;
    chunk->updateIndex = _loadList.size();
    _loadList.push_back(chunk);
}

void ChunkManager::addToGenerateList(Chunk* chunk) {
    chunk->_state = ChunkStates::GENERATE;
    chunk->setupListPtr = &_generateList;
    chunk->updateIndex = _generateList.size();
    _generateList.push_back(chunk);
}

void ChunkManager::addToMeshList(Chunk* chunk) {
    chunk->setupListPtr = &_meshList;
    chunk->updateIndex = _meshList.size();
    _meshList.push_back(chunk);
}

void ChunkManager::recycleChunk(Chunk* chunk) {
    _freeList.push_back(chunk);
    _chunkDiagnostics.numAwaitingReuse++;
}

inline Chunk* ChunkManager::produceChunk() {
    if (_freeList.size()) {
        _chunkDiagnostics.numAwaitingReuse--;
        Chunk* rv = _freeList.back();
        _freeList.pop_back();
        return rv;
    }
    _chunkDiagnostics.numCurrentlyAllocated++;
    _chunkDiagnostics.totalAllocated++;
    return new Chunk();
}

void ChunkManager::deleteAllChunks() {
    for (i32 i = 0; i < _freeList.size(); i++) {
        _chunkDiagnostics.numAwaitingReuse--;
        _chunkDiagnostics.numCurrentlyAllocated--;
        _chunkDiagnostics.totalFreed++;
        delete _freeList[i];
    }
    _freeList.clear();
}

i32v3 ChunkManager::getChunkPosition(const f64v3& position) {
    //Based on the position, calculate a corner position for the chunk
    
    i32v3 rval;
    rval.x = (int)fastFloor(position.x / (double)CHUNK_WIDTH);
    rval.y = (int)fastFloor(position.y / (double)CHUNK_WIDTH);
    rval.z = (int)fastFloor(position.z / (double)CHUNK_WIDTH);
    return rval;
}

void ChunkManager::updateChunks(const f64v3& position) {
    ChunkSlot *cs;
    Chunk* chunk;

    i32v3 chPos;
    i32v3 intPosition(position);

    //ui32 sticks = SDL_GetTicks();

    static ui32 saveTicks = SDL_GetTicks();

    bool save = 0;

    if (SDL_GetTicks() - saveTicks >= 60000 && !generateOnly) { //save once per minute
        save = 1;
        cout << "SAVING\n";
        saveTicks = SDL_GetTicks();
    }

    for (i32 i = (i32)_chunkSlots[0].size()-1; i >= 0; i--) { //update distances for all chunks
        cs = &(_chunkSlots[0][i]);

        cs->calculateDistance2(intPosition);
        chunk = cs->chunk;

        if (cs->distance2 > (graphicsOptions.voxelRenderDistance + 36) * (graphicsOptions.voxelRenderDistance + 36)) { //out of maximum range
            if (!generateOnly && chunk->dirty && chunk->_state > ChunkStates::TREES) {
                GameManager::chunkIOManager->addToSaveList(cs->chunk);
            }
            freeChunk(chunk);
            cs->chunk = nullptr;
            _chunkSlots[0][i].clearNeighbors(_chunkSlots[0]);
            _chunkSlots[0][i] = _chunkSlots[0].back();
            _chunkSlots[0][i].vecIndex = i;
            _chunkSlotIndexMap[_chunkSlots[0][i].position] = i;
            _chunkSlots[0].pop_back();
        } else { //inside maximum range

            //See if neighbors need to be added
            if (cs->numNeighbors != 6) {
                updateChunkslotNeighbors(cs, intPosition);
            }

            //Calculate the LOD as a power of two
            int newLOD = (int)(sqrt(chunk->distance2) / graphicsOptions.voxelLODThreshold) + 1;
            //  newLOD = 2;
            if (newLOD > 6) newLOD = 6;
            if (newLOD != chunk->getLevelOfDetail()) {
                chunk->setLevelOfDetail(newLOD);
                chunk->changeState(ChunkStates::MESH);
            }

            if (isWaterUpdating && chunk->mesh != nullptr) ChunkUpdater::randomBlockUpdates(chunk);

            //calculate light stuff: THIS CAN LAG THE GAME
            if (chunk->_state > ChunkStates::TREES) {
                if (chunk->sunRemovalList.size()) {
                    VoxelLightEngine::calculateSunlightRemoval(chunk);
                }
                if (chunk->sunExtendList.size()) {
                    VoxelLightEngine::calculateSunlightExtend(chunk);
                }
                VoxelLightEngine::calculateLight(chunk);
            }
            //Check to see if it needs to be added to the mesh list
            if (chunk->setupListPtr == nullptr && chunk->inRenderThread == false) {
                switch (chunk->_state) {
                case ChunkStates::WATERMESH:
                case ChunkStates::MESH:
                    addToMeshList(chunk);
                    break;
                }
            }

            //save if its been a minute
            if (save && chunk->dirty) {
                GameManager::chunkIOManager->addToSaveList(chunk);  
            }
        }
    }
}

void ChunkManager::updateChunkslotNeighbors(ChunkSlot* cs, const i32v3& cameraPos) {
    ChunkSlot* newCs;
    if (cs->left == -1) {
        newCs = tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(-1, 0, 0));
        if (newCs) {
            cs->left = newCs->vecIndex;
            newCs->right = cs->vecIndex;
            newCs->detectNeighbors(_chunkSlots[0], _chunkSlotIndexMap);
        }
    }
    if (cs->right == -1) {
        newCs = tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(1, 0, 0));
        if (newCs) {
            cs->right = newCs->vecIndex;
            newCs->left = cs->vecIndex;
            newCs->detectNeighbors(_chunkSlots[0], _chunkSlotIndexMap);
        }
    }
    if (cs->back == -1) {
        newCs = tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(0, 0, -1));
        if (newCs) {
            cs->back = newCs->vecIndex;
            newCs->front = cs->vecIndex;
            newCs->detectNeighbors(_chunkSlots[0], _chunkSlotIndexMap);
        }
    }
    if (cs->front == -1) {
        newCs = tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(0, 0, 1));
        if (newCs) {
            cs->front = newCs->vecIndex;
            newCs->back = cs->vecIndex;
            newCs->detectNeighbors(_chunkSlots[0], _chunkSlotIndexMap);
        }
    }
    if (cs->bottom == -1) {
        newCs = tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(0, -1, 0));
        if (newCs) {
            cs->bottom = newCs->vecIndex;
            newCs->top = cs->vecIndex;
            newCs->detectNeighbors(_chunkSlots[0], _chunkSlotIndexMap);
        }
    }
    if (cs->top == -1) {
        newCs = tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(0, 1, 0));
        if (newCs) {
            cs->top = newCs->vecIndex;
            newCs->bottom = cs->vecIndex;
            newCs->detectNeighbors(_chunkSlots[0], _chunkSlotIndexMap);
        }
    }
}

ChunkSlot* ChunkManager::tryLoadChunkslotNeighbor(ChunkSlot* cs, const i32v3& cameraPos, const i32v3& offset) {
    i32v3 newPosition = cs->position + offset * CHUNK_WIDTH;
   
    double dist2 = ChunkSlot::getDistance2(newPosition, cameraPos);
    double dist = sqrt(dist2);
    if (dist2 <= (graphicsOptions.voxelRenderDistance + CHUNK_WIDTH) * (graphicsOptions.voxelRenderDistance + CHUNK_WIDTH)) {

        i32v2 gridPosition(newPosition.x, newPosition.y);
        ChunkGridData* chunkGridData = getChunkGridData(gridPosition);
        if (chunkGridData == nullptr) {
            chunkGridData = new ChunkGridData(cs->faceData->face, 
                                              cs->faceData->ipos + offset.z, 
                                              cs->faceData->jpos + offset.z, 
                                              cs->faceData->rotation);
            _chunkGridDataMap[gridPosition] = chunkGridData;
        } else {
            chunkGridData->refCount++;
        }

        _chunkSlots[0].emplace_back(newPosition, nullptr, cs->ipos + offset.z, cs->jpos + offset.x, &chunkGridData->faceData);

        ChunkSlot* newcs = &_chunkSlots[0].back();
        newcs->numNeighbors = 1;
        newcs->vecIndex = _chunkSlots[0].size() - 1;

        Chunk* chunk = produceChunk();
        chunk->init(newcs->position, newcs);
        chunk->distance2 = newcs->distance2;
        chunk->chunkGridData = chunkGridData;
        newcs->chunk = chunk;
        addToLoadList(chunk);
        // cout << _chunkHashMap.size() << " ";
        // fflush(stdout);
        Chunk* ch = getChunk(chunk->chunkPosition);
        if (ch != nullptr) {
            cout << "WHOOPSIEDAISY\n";
            fflush(stdout);
        }
        _chunkSlotIndexMap[chunk->chunkPosition] = newcs->vecIndex;
        cs->numNeighbors++;
        return newcs;
        
    }
    return nullptr;
}

void ChunkManager::saveAllChunks() {
    if (generateOnly) return;
    Chunk* chunk;
    for (i32 i = 0; i < _chunkSlots[0].size(); i++) { //update distances for all chunks
        chunk = _chunkSlots[0][i].chunk;
        if (chunk && chunk->dirty && chunk->_state > ChunkStates::TREES) {
            GameManager::chunkIOManager->addToSaveList(chunk);
        }
    }
}

void ChunkManager::prepareHeightMap(HeightData heightData[CHUNK_LAYER]) {
    i32 minNearHeight; //to determine if we should remove surface blocks
    i32 heightDiffThreshold = 3;
    i32 tmp;
    i32 maph;
    Biome *biome;
    i32 sandDepth, snowDepth;
    for (i32 z = 0; z < CHUNK_WIDTH; z++) {
        for (i32 x = 0; x < CHUNK_WIDTH; x++) {

            //*************Calculate if it is too steep ***********************
            maph = heightData[z*CHUNK_WIDTH + x].height;
            biome = heightData[z*CHUNK_WIDTH + x].biome;
            sandDepth = heightData[z*CHUNK_WIDTH + x].sandDepth;
            snowDepth = heightData[z*CHUNK_WIDTH + x].snowDepth;

            minNearHeight = maph;
            if (x > 0) { //Could sentinalize this in the future
                minNearHeight = heightData[z*CHUNK_WIDTH + x - 1].height;
            } else {
                minNearHeight = maph + (maph - heightData[z*CHUNK_WIDTH + x + 1].height); //else use opposite side but negating the difference
            }
            if (x < CHUNK_WIDTH - 1) {
                tmp = heightData[z*CHUNK_WIDTH + x + 1].height;
            } else {
                tmp = maph + (maph - heightData[z*CHUNK_WIDTH + x - 1].height);
            }
            if (tmp < minNearHeight) minNearHeight = tmp;
            if (z > 0) {
                tmp = heightData[(z - 1)*CHUNK_WIDTH + x].height;
            } else {
                tmp = maph + (maph - heightData[(z + 1)*CHUNK_WIDTH + x].height);
            }
            if (tmp < minNearHeight) minNearHeight = tmp;
            if (z < CHUNK_WIDTH - 1) {
                tmp = heightData[(z + 1)*CHUNK_WIDTH + x].height;
            } else {
                tmp = maph + (maph - heightData[(z - 1)*CHUNK_WIDTH + x].height);
            }
            if (tmp < minNearHeight) minNearHeight = tmp;

            if (maph - minNearHeight >= heightDiffThreshold && heightData[z*CHUNK_WIDTH + x].biome->looseSoilDepth) {
                heightData[z*CHUNK_WIDTH + x].flags |= TOOSTEEP;
            }
            //*************End TOOSTEEP calculation ***********************

            //**************START SURFACE BLOCK CALCULATION******************
            GLushort surfaceBlock = DIRT;
            if (maph == 0) {
                surfaceBlock = biome->beachBlock;
            } else if (maph < 0) {
                surfaceBlock = biome->underwaterBlock;
            } else {
                surfaceBlock = biome->surfaceBlock;
                if (surfaceBlock == DIRTGRASS && (snowDepth || sandDepth)) surfaceBlock = DIRT;
            }
            heightData[z*CHUNK_WIDTH + x].surfaceBlock = surfaceBlock;

            //**************END SURFACE BLOCK CALCULATION******************
        }
    }
}

i32 ChunkManager::getClosestChunks(f64v3 &coords, Chunk** chunks) {
#define GETCHUNK(y, z, x) it = _chunkSlotIndexMap.find(chPos + i32v3(x, y, z)); chunk = (it == _chunkSlotIndexMap.end() ? nullptr : chunkSlots[it->second].chunk);

    i32 xDir, yDir, zDir;
    Chunk* chunk;

    

    std::unordered_map<i32v3, int>::iterator it;
    vector <ChunkSlot>& chunkSlots = _chunkSlots[0];

    f64v3 relativePos = coords - f64v3(cornerPosition);

    //Get the chunk coordinates (assume its always positive)
    i32v3 chPos;
    chPos.x = (i32)relativePos.x / CHUNK_WIDTH;
    chPos.y = (i32)relativePos.y / CHUNK_WIDTH;
    chPos.z = (i32)relativePos.z / CHUNK_WIDTH;

    //Determines if were closer to positive or negative chunk
    xDir = (relativePos.x - chPos.x > 0.5f) ? 1 : -1;
    yDir = (relativePos.y - chPos.y > 0.5f) ? 1 : -1;
    zDir = (relativePos.z - chPos.z > 0.5f) ? 1 : -1;

    //clear the memory for the chunk pointer array
    chunks[0] = chunks[1] = chunks[2] = chunks[3] = chunks[4] = chunks[5] = chunks[6] = chunks[7] = nullptr;

    //If the 8 nnearby exist and are accessible then set them in the array. NOTE: Perhaps order matters? 

    GETCHUNK(0, 0, 0);
    if (chunk && chunk->isAccessible) chunks[0] = chunk;
    GETCHUNK(0, 0, xDir);
    if (chunk && chunk->isAccessible) chunks[1] = chunk;
    GETCHUNK(0, zDir, 0);
    if (chunk && chunk->isAccessible) chunks[2] = chunk;
    GETCHUNK(0, zDir, xDir);
    if (chunk && chunk->isAccessible) chunks[3] = chunk;
    GETCHUNK(yDir, 0, 0);
    if (chunk && chunk->isAccessible) chunks[4] = chunk;
    GETCHUNK(yDir, 0, xDir);
    if (chunk && chunk->isAccessible) chunks[5] = chunk;
    GETCHUNK(yDir, zDir, 0);
    if (chunk && chunk->isAccessible) chunks[6] = chunk;
    GETCHUNK(yDir, zDir, xDir);
    if (chunk && chunk->isAccessible) chunks[7] = chunk;
    return 1;
    
    return 0;
}

void ChunkManager::clearChunkFromLists(Chunk* chunk) {
    i32 dindex = chunk->drawIndex;
    i32 uindex = chunk->updateIndex;
    boost::circular_buffer<Chunk*> *sp;

    chunk->clearBuffers();

    if (uindex != -1 || chunk->setupListPtr != nullptr) { //load and render
        sp = chunk->setupListPtr;
        (*sp)[uindex] = sp->back();
        (*sp)[uindex]->updateIndex = uindex;
        sp->pop_back();

        chunk->updateIndex = -1;
        chunk->setupListPtr = nullptr;
    }

    if (chunk->inFinishedChunks) { //it is in finished chunks queue
        taskQueueManager.fcLock.lock();
        for (size_t i = 0; i < taskQueueManager.finishedChunks.size(); i++) {
            if (taskQueueManager.finishedChunks[i] == chunk) {
                taskQueueManager.finishedChunks[i] = taskQueueManager.finishedChunks.back();
                chunk->inFinishedChunks = 0;
                chunk->inGenerateThread = 0;
                taskQueueManager.finishedChunks.pop_back();
                break;
            }
        }
        taskQueueManager.fcLock.unlock();
    }
    if (chunk->inFinishedMeshes) { //it is in finished meshes queue
        taskQueueManager.frLock.lock();
        for (size_t i = 0; i < taskQueueManager.finishedChunkMeshes.size(); i++) {
            if (taskQueueManager.finishedChunkMeshes[i]->chunk == chunk) {
                delete taskQueueManager.finishedChunkMeshes[i];
                taskQueueManager.finishedChunkMeshes[i] = taskQueueManager.finishedChunkMeshes.back();
                chunk->inFinishedMeshes = 0;
                chunk->inRenderThread = 0;
                taskQueueManager.finishedChunkMeshes.pop_back();
                break;
            }
        }
        taskQueueManager.frLock.unlock();
    }

    chunk->clearNeighbors();


    _chunkSlotIndexMap.erase(chunk->chunkPosition);
}

void ChunkManager::clearChunk(Chunk* chunk) {

    clearChunkFromLists(chunk);
    chunk->clearBuffers();
   
    chunk->freeWaiting = false;
    chunk->clear();

    chunk->inSaveThread = 0;
    chunk->inLoadThread = 0;
}

void ChunkManager::recursiveSortChunks(boost::circular_buffer<Chunk*> &v, i32 start, i32 size, i32 type) {
    if (size < 2) return;
    i32 i, j;
    Chunk* pivot, *mid, *last, *tmp;

    pivot = v[start];

    //end recursion when small enough
    if (size == 2) {
        if (pivot->distance2 < v[start + 1]->distance2) {
            v[start] = v[start + 1];
            v[start + 1] = pivot;
            if (type == 1) {
                v[start]->drawIndex = start;
                v[start + 1]->drawIndex = start + 1;
            } else {
                v[start]->updateIndex = start;
                v[start + 1]->updateIndex = start + 1;
            }
        }
        return;
    }

    mid = v[start + size / 2];
    last = v[start + size - 1];

    //variables to minimize dereferences
    i32 md, ld, pd;
    pd = pivot->distance2;
    md = mid->distance2;
    ld = last->distance2;

    //calculate pivot
    if ((pd > md && md > ld) || (pd < md && md < ld)) {
        v[start] = mid;

        v[start + size / 2] = pivot;

        if (type == 1) {
            mid->drawIndex = start;
            pivot->drawIndex = start + size / 2;
        } else {
            mid->updateIndex = start;
            pivot->updateIndex = start + size / 2;
        }

        pivot = mid;
        pd = md;
    } else if ((pd > ld && ld > md) || (pd < ld && ld < md)) {
        v[start] = last;

        v[start + size - 1] = pivot;

        if (type == 1) {
            last->drawIndex = start;
            pivot->drawIndex = start + size - 1;
        } else {
            last->updateIndex = start;
            pivot->updateIndex = start + size - 1;
        }

        pivot = last;
        pd = ld;
    }

    i = start + 1;
    j = start + size - 1;

    //increment and decrement pointers until they are past each other
    while (i <= j) {
        while (i < start + size - 1 && v[i]->distance2 > pd) i++;
        while (j > start + 1 && v[j]->distance2 < pd) j--;

        if (i <= j) {
            tmp = v[i];
            v[i] = v[j];
            v[j] = tmp;

            if (type == 1) {
                v[i]->drawIndex = i;
                v[j]->drawIndex = j;
            } else {
                v[i]->updateIndex = i;
                v[j]->updateIndex = j;
            }

            i++;
            j--;
        }
    }

    //swap pivot with rightmost element in left set
    v[start] = v[j];
    v[j] = pivot;

    if (type == 1) {
        v[start]->drawIndex = start;
        v[j]->drawIndex = j;
    } else {
        v[start]->updateIndex = start;
        v[j]->updateIndex = j;
    }

    //sort the two subsets excluding the pivot point
    recursiveSortChunks(v, start, j - start, type);
    recursiveSortChunks(v, j + 1, start + size - j - 1, type);
}

bool ChunkManager::isChunkPositionInBounds(const i32v3& position) const {
    return !(position.x < 0 || position.x >= csGridWidth || position.y < 0 || position.y >= csGridWidth || position.z < 0 || position.z >= csGridWidth);
}

//Lookup a chunk and block index for a specific integer position in the world. chunk will be NULL if none is found.
void ChunkManager::getBlockAndChunk(const i32v3& relativePosition, Chunk** chunk, int& blockIndex) const {

    //Get the chunk coordinates
    i32v3 chunkPosition = relativePosition / CHUNK_WIDTH;

    //Bounds checking

    auto it = _chunkSlotIndexMap.find(chunkPosition);
    if (it == _chunkSlotIndexMap.end()) {
        *chunk = nullptr;
        return;
    }

    *chunk = _chunkSlots[0][it->second].chunk;

    if (chunk) {
        //reuse chunkPosition variable to get the position relative to owner chunk
        chunkPosition = relativePosition - chunkPosition * CHUNK_WIDTH;

        blockIndex = chunkPosition.y * CHUNK_LAYER + chunkPosition.z * CHUNK_WIDTH + chunkPosition.x;
    }
}

void ChunkManager::remeshAllChunks() {
    Chunk* chunk;
    for (int i = 0; i < _chunkSlots[0].size(); i++) {
        chunk = _chunkSlots[0][i].chunk;
        if (chunk) {
            chunk->changeState(ChunkStates::MESH);
        }
    }
}

void ChunkManager::recursiveSortSetupList(boost::circular_buffer<Chunk*>& v, i32 start, i32 size, i32 type) {
    if (size < 2) return;
    i32 i, j;
    Chunk* pivot, *mid, *last, *tmp;

    pivot = v[start];

    //end recursion when small enough
    if (size == 2) {
        if ((pivot->distance2 - pivot->setupWaitingTime) < (v[start + 1]->distance2 - v[start + 1]->setupWaitingTime)) {
            v[start] = v[start + 1];
            v[start + 1] = pivot;
            if (type == 1) {
                v[start]->drawIndex = start;
                v[start + 1]->drawIndex = start + 1;
            } else {
                v[start]->updateIndex = start;
                v[start + 1]->updateIndex = start + 1;
            }
        }
        return;
    }

    mid = v[start + size / 2];
    last = v[start + size - 1];

    //variables to minimize dereferences
    i32 md, ld, pd;
    pd = pivot->distance2 - pivot->setupWaitingTime;
    md = mid->distance2 - mid->setupWaitingTime;
    ld = last->distance2 - last->setupWaitingTime;

    //calculate pivot
    if ((pd > md && md > ld) || (pd < md && md < ld)) {
        v[start] = mid;

        v[start + size / 2] = pivot;

        if (type == 1) {
            mid->drawIndex = start;
            pivot->drawIndex = start + size / 2;
        } else {
            mid->updateIndex = start;
            pivot->updateIndex = start + size / 2;
        }

        pivot = mid;
        pd = md;
    } else if ((pd > ld && ld > md) || (pd < ld && ld < md)) {
        v[start] = last;

        v[start + size - 1] = pivot;

        if (type == 1) {
            last->drawIndex = start;
            pivot->drawIndex = start + size - 1;
        } else {
            last->updateIndex = start;
            pivot->updateIndex = start + size - 1;
        }

        pivot = last;
        pd = ld;
    }

    i = start + 1;
    j = start + size - 1;

    //increment and decrement pointers until they are past each other
    while (i <= j) {
        while (i < start + size - 1 && (v[i]->distance2 - v[i]->setupWaitingTime) > pd) i++;
        while (j > start + 1 && (v[j]->distance2 - v[j]->setupWaitingTime) < pd) j--;

        if (i <= j) {
            tmp = v[i];
            v[i] = v[j];
            v[j] = tmp;

            if (type == 1) {
                v[i]->drawIndex = i;
                v[j]->drawIndex = j;
            } else {
                v[i]->updateIndex = i;
                v[j]->updateIndex = j;
            }

            i++;
            j--;
        }
    }

    //swap pivot with rightmost element in left set
    v[start] = v[j];
    v[j] = pivot;

    if (type == 1) {
        v[start]->drawIndex = start;
        v[j]->drawIndex = j;
    } else {
        v[start]->updateIndex = start;
        v[j]->updateIndex = j;
    }

    //sort the two subsets excluding the pivot point
    recursiveSortSetupList(v, start, j - start, type);
    recursiveSortSetupList(v, j + 1, start + size - j - 1, type);
}

i32 ChunkManager::getBlockFromDir(f64v3& dir, f64v3& pos) {
    static const PredBlockID predBlock = [] (const i32& id) {
        return id && (id < LOWWATER || id > FULLWATER);
    };
    return VRayHelper::getQuery(pos, f32v3(dir), this, predBlock).id;
}

void ChunkManager::caveOcclusion(const f64v3& ppos) {
    static ui32 frameCounter = 0;
    frameCounter++;

    i32v3 chPos;
    Chunk* ch;

    chPos.x = (ppos.x - cornerPosition.x) / CHUNK_WIDTH;
    chPos.y = (ppos.y - cornerPosition.y) / CHUNK_WIDTH;
    chPos.z = (ppos.z - cornerPosition.z) / CHUNK_WIDTH;

    if (frameCounter == 10 || chPos.x != _poccx || chPos.y != _poccy || chPos.z != _poccz) {
        _poccx = chPos.x;
        _poccy = chPos.y;
        _poccz = chPos.z;

        for (i32 i = 0; i < _chunkSlots[0].size(); i++) {
            if (_chunkSlots[0][i].chunk) _chunkSlots[0][i].chunk->occlude = 1;
        }

        ch = getChunk(chPos);
        if ((!ch) || ch->isAccessible == false) return;

        recursiveFloodFill(1, 1, 1, 1, 1, 1, ch);
    }
}

//0 1 2
//7   3
//6 5 4

void ChunkManager::recursiveFloodFill(bool left, bool right, bool front, bool back, bool top, bool bottom, Chunk* chunk) {
    if (chunk->_state == ChunkStates::LOAD || chunk->occlude == 0) return;
    chunk->occlude = 0;
    if (left && chunk->left && chunk->leftBlocked == 0) {
        if (chunk->left->rightBlocked == 0) {
            recursiveFloodFill(left, 0, front, back, top, bottom, chunk->left);
        } else {
            chunk->left->occlude = 0;
        }
    }
    if (right && chunk->right && chunk->rightBlocked == 0) {
        if (chunk->right->leftBlocked == 0) {
            recursiveFloodFill(0, right, front, back, top, bottom, chunk->right);
        } else {
            chunk->right->occlude = 0;
        }
    }
    if (front && chunk->front && chunk->frontBlocked == 0) {
        if (chunk->front->backBlocked == 0) {
            recursiveFloodFill(left, right, front, 0, top, bottom, chunk->front);
        } else {
            chunk->front->occlude = 0;
        }
    }
    if (back && chunk->back && chunk->backBlocked == 0) {
        if (chunk->back->frontBlocked == 0) {
            recursiveFloodFill(left, right, 0, back, top, bottom, chunk->back);
        } else {
            chunk->back->occlude = 0;
        }
    }
    if (top && chunk->top && chunk->topBlocked == 0) {
        if (chunk->top->bottomBlocked == 0) {
            recursiveFloodFill(left, right, front, back, top, 0, chunk->top);
        } else {
            chunk->top->occlude = 0;
        }
    }
    if (bottom && chunk->bottom && chunk->bottomBlocked == 0) {
        if (chunk->bottom->topBlocked == 0) {
            recursiveFloodFill(left, right, front, back, 0, bottom, chunk->bottom);
        } else {
            chunk->bottom->occlude = 0;
        }
    }
}

//returns 0 on success, 1 on fail
i32 ChunkManager::getPositionHeightData(i32 posX, i32 posZ, HeightData& hd) {
    //player biome
    i32v2 gridPosition(fastFloor(posX / (float)CHUNK_WIDTH) * CHUNK_WIDTH, fastFloor(posZ / (float)CHUNK_WIDTH) * CHUNK_WIDTH);
    ChunkGridData* chunkGridData = getChunkGridData(gridPosition);
    if (chunkGridData) {
        if (chunkGridData->heightData[0].height == UNLOADED_HEIGHT) return 1;
        hd = chunkGridData->heightData[(posZ%CHUNK_WIDTH) * CHUNK_WIDTH + posX%CHUNK_WIDTH];
        return 0;
    } else {
        return 1; //fail
    }
}

Chunk* ChunkManager::getChunk(const f64v3& position) {

    i32v3 chPos;
    chPos = i32v3((position - f64v3(cornerPosition)) / (double)CHUNK_WIDTH);

    auto it = _chunkSlotIndexMap.find(chPos);
    if (it == _chunkSlotIndexMap.end()) return nullptr;
    return _chunkSlots[0][it->second].chunk;

}

Chunk* ChunkManager::getChunk(const i32v3& worldPos) {
    auto it = _chunkSlotIndexMap.find(worldPos);
    if (it == _chunkSlotIndexMap.end()) return nullptr;
    return _chunkSlots[0][it->second].chunk;
}

const Chunk* ChunkManager::getChunk(const i32v3& worldPos) const {
    auto it = _chunkSlotIndexMap.find(worldPos);
    if (it == _chunkSlotIndexMap.end()) return nullptr;
    return _chunkSlots[0][it->second].chunk;
}

ChunkGridData* ChunkManager::getChunkGridData(const i32v2& gridPos) {
    auto it = _chunkGridDataMap.find(gridPos);
    if (it == _chunkGridDataMap.end()) return nullptr;
    return it->second;
}

void ChunkManager::removeFromSetupList(Chunk* ch) {

}

const i16* ChunkManager::getIDQuery(const i32v3& start, const i32v3& end) const {
    i32v3 pIter = start;

    // Create The Array For The IDs
    const i32v3 size = end - start + i32v3(1);
    i32 volume = size.x * size.y * size.z;
    i16* q = new i16[volume];

    i32 i = 0;
    i32v3 chunkPos, voxelPos;
    for (; pIter.y <= end.y; pIter.y++) {
        for (pIter.z = start.z; pIter.z <= end.z; pIter.z++) {
            for (pIter.x = start.x; pIter.x <= end.x; pIter.x++) {
                // Get The Chunk
                chunkPos = pIter / CHUNK_WIDTH;
                const Chunk* c = getChunk(chunkPos);

                // Get The ID
                voxelPos = pIter % CHUNK_WIDTH;
                q[i++] = c->getBlockID(voxelPos.y * CHUNK_LAYER + voxelPos.z * CHUNK_WIDTH + voxelPos.x);
            }
        }
    }

    openglManager.debugRenderer->drawCube(
        f32v3(start + end) * 0.5f + f32v3(cornerPosition) + f32v3(0.5f), f32v3(size) + f32v3(0.4f),
        f32v4(1.0f, 0.0f, 0.0f, 0.3f), 1.0f
        );

    return q;
}
