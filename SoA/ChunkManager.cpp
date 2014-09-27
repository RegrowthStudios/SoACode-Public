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

void ChunkManager::initialize(const f64v3& gridPosition, FaceData *playerFaceData, ui32 flags) {
    _playerFace = playerFaceData;
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
    _allChunkSlots = new ChunkSlot[csGridSize];

    _hz = 0;
    _hx = 0;

    calculateCornerPosition(gpos);

    initializeHeightMap();

    if (flags & FLAT_GRASS) {
        for (size_t i = 0; i < _heightMap.size(); i++) {
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
        }
    }

    i32 mid = csGridWidth / 2;

    //flatgrass map
    if (flags & SET_Y_TO_SURFACE) {
        if (!(flags & FLAT_GRASS)) {//generate a single height so we know where the player should go
            currTerrainGenerator->GenerateHeightMap(_heightMap[mid][mid], (_faceMap[mid][mid]->ipos*CHUNK_WIDTH - planet->radius), (_faceMap[mid][mid]->jpos*CHUNK_WIDTH - planet->radius), CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, 1, 0);
            prepareHeightMap(_heightMap[mid][mid], 0, 0, CHUNK_WIDTH, CHUNK_WIDTH);

        }

        i32 yPos = _heightMap[mid][mid][CHUNK_LAYER / 2].height + _heightMap[mid][mid][CHUNK_LAYER / 2].snowDepth + _heightMap[mid][mid][CHUNK_LAYER / 2].sandDepth + 15;
        if (yPos < 2) yPos = 2; //keeps player from spawning at the bottom of the ocean

        cornerPosition.y = (i32)floor((yPos - CHUNK_WIDTH * csGridWidth / 2.0) / 32.0) * 32;
    }
}

void ChunkManager::resizeGrid(const f64v3& gpos) {
    //DrawLoadingScreen("Resizing Chunk Grid...", false, glm::vec4(0.0, 0.0, 0.0, 0.3));

    clearAllChunks(1);

    for (i32 y = 0; y < csGridWidth; y++) {
        for (i32 z = 0; z < csGridWidth; z++) {
            for (i32 x = 0; x < csGridWidth; x++) {
                freeChunk(_chunkList[y][z][x]->chunk);
            }
        }
    }

    for (size_t i = 0; i < _threadWaitingChunks.size(); i++) { //kill the residual waiting threads too
        recycleChunk(_threadWaitingChunks[i]);
    }
    _threadWaitingChunks.clear();

    delete[] _allChunkSlots;

    initializeGrid(gpos, 0);
    InitializeChunks();
    loadAllChunks(2, gpos);
}

void ChunkManager::initializeHeightMap() {
    _heightMap.resize(csGridWidth);
    _faceMap.resize(csGridWidth);
    for (ui32 i = 0; i < _heightMap.size(); i++) {
        _heightMap[i].resize(csGridWidth);
        _faceMap[i].resize(csGridWidth);
    }

    i32 rot = _playerFace->rotation;
    i32 istrt = (cornerPosition.z + planet->radius) / CHUNK_WIDTH;
    i32 jstrt = (cornerPosition.x + planet->radius) / CHUNK_WIDTH;
    i32 face = _playerFace->face;


    i32 ipos = FaceCoords[face][rot][0];
    i32 jpos = FaceCoords[face][rot][1];
    i32 rpos = FaceCoords[face][rot][2];
    i32 idir = FaceOffsets[face][rot][0];
    i32 jdir = FaceOffsets[face][rot][1];

    currTerrainGenerator->SetLODFace(ipos, jpos, rpos, FaceRadSign[face] * planet->radius, idir, jdir, 1.0);

    for (i32 i = 0; i < csGridWidth; i++) {
        for (i32 j = 0; j < csGridWidth; j++) {
            //TODO: Store as POD? (instead of new)
            _faceMap[i][j] = new FaceData();
            _faceMap[i][j]->Set(face, istrt + i, jstrt + j, rot);
            _heightMap[i][j] = new HeightData[CHUNK_LAYER];
            for (i32 k = 0; k < CHUNK_LAYER; k++) {
                _heightMap[i][j][k].height = UNLOADED_HEIGHT;
            }
            /*currTerrainGenerator->GenerateHeightMap(heightMap[i][j], Z+i*chunkWidth, X+j*chunkWidth, chunkWidth, chunkWidth, chunkWidth, 1, 0);
            PrepareHeightMap(heightMap[i][j], 0, 0, chunkWidth, chunkWidth);*/
        }
    }
}

void ChunkManager::regenerateHeightMap(i32 loadType) {
    i32 mid = (csGridWidth*CHUNK_WIDTH) / 2;
    i32 istrt = (cornerPosition.z + planet->radius) / CHUNK_WIDTH;
    i32 jstrt = (cornerPosition.x + planet->radius) / CHUNK_WIDTH;
    for (i32 i = 0; i < csGridWidth; i++) {
        for (i32 j = 0; j < csGridWidth; j++) {
            _faceMap[i][j]->Set(0, istrt + i, jstrt + j, 0);
        }
    }

    currTerrainGenerator->SetLODFace(2, 0, 1, planet->radius);

    if (loadType == 2) { //dont load whole thing
        for (size_t i = 0; i < _heightMap.size(); i++) {
            for (size_t j = 0; j < _heightMap[j].size(); j++) {
                for (i32 k = 0; k < CHUNK_LAYER; k++) {
                    _heightMap[i][j][k].height = UNLOADED_HEIGHT; //flag for reload
                }
            }
        }
        for (i32 i = 0; i < csGridWidth; i++) {
            for (i32 j = 0; j < csGridWidth; j++) {
                currTerrainGenerator->GenerateHeightMap(_heightMap[i][j], (cornerPosition.z + i), (cornerPosition.x + j)*CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, 1, 0);
                prepareHeightMap(_heightMap[i][j], 0, 0, CHUNK_WIDTH, CHUNK_WIDTH);
            }
        }
    } else { //load the whole thing
        for (i32 i = 0; i < csGridWidth; i++) {
            for (i32 j = 0; j < csGridWidth; j++) {
                currTerrainGenerator->GenerateHeightMap(_heightMap[i][j], (cornerPosition.z + i)*CHUNK_WIDTH, (cornerPosition.x + j)*CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, 1, 0);
                prepareHeightMap(_heightMap[i][j], 0, 0, CHUNK_WIDTH, CHUNK_WIDTH);
            }
        }
    }
}

void ChunkManager::InitializeChunks() {
    i32 c = 0;
    _chunkList.resize(csGridWidth);
    for (i32 y = 0; y < csGridWidth; y++) {
        _chunkList[y].resize(csGridWidth);
        for (i32 z = 0; z < csGridWidth; z++) {
            _chunkList[y][z].resize(csGridWidth);
            for (i32 x = 0; x < csGridWidth; x++) {
                _chunkList[y][z][x] = &(_allChunkSlots[c++]);
                _chunkList[y][z][x]->Initialize(cornerPosition + glm::ivec3(x, y, z) * CHUNK_WIDTH, NULL, z, x, _faceMap[z][x]);
            }
        }
    }
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
        for (i32 i = 0; i < csGridSize; i++) {
            if (_allChunkSlots[i].chunk) _allChunkSlots[i].chunk->clear();
        }
    } else {
        for (i32 i = 0; i < csGridSize; i++) {
            if (_allChunkSlots[i].chunk) _allChunkSlots[i].chunk->clear(0);
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

    for (i32 z = 0; z < csGridWidth; z++) {
        for (i32 x = 0; x < csGridWidth; x++) {
            delete _faceMap[z][x];
            delete[] _heightMap[z][x];
        }
    }
    for (i32 y = 0; y < csGridWidth; y++) {
        for (i32 z = 0; z < csGridWidth; z++) {
            for (i32 x = 0; x < csGridWidth; x++) {
                freeChunk(_chunkList[y][z][x]->chunk);
            }
        }
    }

    GameManager::physicsEngine->clearAll();

    for (size_t i = 0; i < _threadWaitingChunks.size(); i++) { //kill the residual waiting threads too
        recycleChunk(_threadWaitingChunks[i]);
    }
    _threadWaitingChunks.clear();

    deleteAllChunks();

    delete[] _allChunkSlots;
}

//loadType 0 = Complete Generation, 1 = Only Load, 2 = only push to queue
void ChunkManager::loadAllChunks(i32 loadType, const f64v3& position) {
    //    tcks = SDL_GetTicks();

    Chunk* chunk;
    ChunkSlot *cs;
    updateChunks(position);

    for (i32 i = 0; i < csGridSize; i++) {
        cs = &(_allChunkSlots[i]);
        chunk = cs->chunk;
        if (chunk != NULL) {
            if (chunk->neighbors != 6) { //prevent residue from carrying over to new biome setup
                chunk->clearBuffers();
            }
            chunk->changeState(ChunkStates::LOAD);
        } else {
            if (cs->distance2 <= (graphicsOptions.voxelRenderDistance + 32) * (graphicsOptions.voxelRenderDistance + 32)) {

                cs->chunk = produceChunk();
                cs->chunk->init(cs->position, cs->ipos, cs->jpos, cs->fd);
            }
        }
    }
    recursiveSortChunks(_loadList, 0, _loadList.size(), 2);
    updateLoadList(100);
    updateLoadedChunks();

}

void ChunkManager::relocateChunks(const f64v3& gpos) {
    //save all chunks
    vector<Chunk*> toSave;
    Chunk* chunk;
    for (i32 i = 0; i < csGridSize; i++) {
        chunk = _allChunkSlots[i].chunk;
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

    i32 rot = _playerFace->rotation;
    i32 istrt = (cornerPosition.z + planet->radius) / CHUNK_WIDTH;
    i32 jstrt = (cornerPosition.x + planet->radius) / CHUNK_WIDTH;
    i32 face = _playerFace->face;

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

    for (i32 z = 0; z < csGridWidth; z++) {
        for (i32 x = 0; x < csGridWidth; x++) {
            _faceMap[z][x]->Set(face, istrt + z, jstrt + x, rot);
        }
    }

    for (i32 y = 0; y < csGridWidth; y++) {
        for (i32 z = 0; z < csGridWidth; z++) {
            for (i32 x = 0; x < csGridWidth; x++) {
                chunk = _chunkList[y][z][x]->chunk;
                if (chunk) {
                    chunk->updateIndex = -1;
                    chunk->drawIndex = -1;
                    chunk->setupListPtr = NULL; //we already cleared the setup lists
                    chunk->inLoadThread = chunk->inSaveThread = 0;
                    freeChunk(chunk);
                }
                _chunkList[y][z][x]->Initialize(cornerPosition + glm::ivec3(x, y, z) * CHUNK_WIDTH, NULL, z, x, _faceMap[z][x]);
            }
        }
    }

    for (Uint32 y = 0; y < _heightMap.size(); y++) {
        for (Uint32 z = 0; z < _heightMap.size(); z++) {
            for (i32 x = 0; x < CHUNK_LAYER; x++) {
                _heightMap[y][z][x].height = UNLOADED_HEIGHT;
            }
        }
    }
}

void ChunkManager::clearChunkData() {

    for (i32 y = 0; y < csGridWidth; y++) {
        for (i32 z = 0; z < csGridWidth; z++) {
            for (i32 x = 0; x < csGridWidth; x++) {
                if (_chunkList[y][z][x]->chunk) {
                    clearChunk(_chunkList[y][z][x]->chunk);
                    recycleChunk(_chunkList[y][z][x]->chunk);
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
    //change the parameter to true to print out the timings
    globalMultiplePreciseTimer.end(true);
}

//traverses the back of the load list, popping of entries and giving them to threads to be loaded
void ChunkManager::updateLoadList(ui32 maxTicks) {
    ui32 sss = SDL_GetTicks();
    i32 depth;
    i32 size = _loadList.size();
    i32 startX, startZ;
    i32 ipos, jpos, rpos;
    i32 idir, jdir;
    i32 rot, face;
    Chunk* chunk;
    vector<Chunk* > chunksToLoad;

    ui32 sticks = SDL_GetTicks();
    for (i32 i = size - 1; i >= 0; i--) {
        chunk = _loadList[i];
        _loadList.pop_back();
        chunk->updateIndex = -1;
        chunk->setupListPtr = NULL;

        startZ = (chunk->hzIndex - _hz);
        startX = (chunk->hxIndex - _hx);
        if (startX < 0 || startZ < 0 || startX >= csGridWidth || startZ >= csGridWidth) {
            pError("Chunk startX or startZ out of bounds");
        }
        //If the heightmap has not been generated, generate it.
        if (_heightMap[startZ][startX][0].height == UNLOADED_HEIGHT) {

            //set up the directions
            rot = _faceMap[startZ][startX]->rotation;
            face = _faceMap[startZ][startX]->face;
            ipos = FaceCoords[face][rot][0];
            jpos = FaceCoords[face][rot][1];
            rpos = FaceCoords[face][rot][2];
            idir = FaceOffsets[face][rot][0];
            jdir = FaceOffsets[face][rot][1];

            currTerrainGenerator->SetLODFace(ipos, jpos, rpos, FaceRadSign[face] * planet->radius, idir, jdir, 1.0);

            currTerrainGenerator->GenerateHeightMap(_heightMap[startZ][startX], (_faceMap[startZ][startX]->ipos*CHUNK_WIDTH - planet->radius), (_faceMap[startZ][startX]->jpos*CHUNK_WIDTH - planet->radius), CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, 1, 0);
            prepareHeightMap(_heightMap[startZ][startX], 0, 0, CHUNK_WIDTH, CHUNK_WIDTH);
        }
        chunk->isAccessible = 0;

        for (i32 i = 0; i < CHUNK_LAYER; i++) {
            chunk->_biomes[i] = _heightMap[startZ][startX][i].biome->vecIndex; // maybe we could bypass this by just giving the chunk a damn pointer rather than copying everything. ugh
            chunk->_rainfalls[i] = _heightMap[startZ][startX][i].rainfall;
            chunk->_temperatures[i] = _heightMap[startZ][startX][i].temperature;

            depth = -_heightMap[startZ][startX][i].height / 5;
            if (depth > 255) {
                depth = 255;
            } else if (depth < 0) {
                depth = 0;
            }
            depth = 255 - depth;
            chunk->_depthMap[i] = depth;
        }

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
            if (chunk->neighbors == 6) {
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

        if (chunk->neighbors == 6 && SphereInFrustum((float)(chunk->position.x + CHUNK_WIDTH / 2 - position.x), (float)(chunk->position.y + CHUNK_WIDTH / 2 - position.y), (float)(chunk->position.z + CHUNK_WIDTH / 2 - position.z), 28.0f, gridFrustum)) {

          
            VoxelLightEngine::calculateLight(chunk);
            
            if (chunk->numBlocks < 0) {
                cout << "CHUNK NUM < 0 == " << chunk->numBlocks << endl;
            }
            //TODO: BEN, Need to make sure chunk->num is always correct
            if (chunk->numBlocks) { 

                chunk->occlude = 0;

                if (chunk->neighbors == 6 && chunk->inRenderThread == 0) {
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
    i32 startX, startZ;

    for (i32 i = _generateList.size() - 1; i >= 0; i--) {
        chunk = _generateList[i];
        _generateList[i] = _generateList.back();
        _generateList[i]->updateIndex = i;

        chunk->updateIndex = -1;
        chunk->setupListPtr = NULL;
        _generateList.pop_back();

        startZ = (chunk->hzIndex - _hz);
        startX = (chunk->hxIndex - _hx);
        if (startX < 0 || startZ < 0 || startZ >= _heightMap.size() || startX >= _heightMap[0].size()) {
            pError("Chunk startX or startZ out of bounds " + to_string(startX) + " " + to_string(startZ));
        }

        chunk->isAccessible = 0;

        threadPool.addLoadJob(chunk, new LoadData(_heightMap[startZ][startX], cornerPosition.x, cornerPosition.z, currTerrainGenerator));

        if (SDL_GetTicks() - startTicks > maxTicks) break;
    }
    return 0;
}

void ChunkManager::setupNeighbors(Chunk* chunk) {
    i32 x = (chunk->position.x - cornerPosition.x) / CHUNK_WIDTH;
    i32 y = (chunk->position.y - cornerPosition.y) / CHUNK_WIDTH;
    i32 z = (chunk->position.z - cornerPosition.z) / CHUNK_WIDTH;
    Chunk* neighbor;

    if (x > 0) { //left
        neighbor = _chunkList[y][z][x - 1]->chunk;
        if (!(chunk->left) && neighbor && neighbor->isAccessible) {
            chunk->left = neighbor;
            chunk->neighbors++;
            chunk->left->right = chunk;
            chunk->left->neighbors++;
        }
    }

    if (x < csGridWidth - 1) { //right
        neighbor = _chunkList[y][z][x + 1]->chunk;
        if (!(chunk->right) && neighbor && neighbor->isAccessible) {
            chunk->right = neighbor;
            chunk->neighbors++;
            chunk->right->left = chunk;
            chunk->right->neighbors++;
        }
    }

    if (z > 0) { //back
        neighbor = _chunkList[y][z - 1][x]->chunk;
        if (!(chunk->back) && neighbor && neighbor->isAccessible) {
            chunk->back = neighbor;
            chunk->neighbors++;
            chunk->back->front = chunk;
            chunk->back->neighbors++;
        }
    }

    if (z < csGridWidth - 1) { //front
        neighbor = _chunkList[y][z + 1][x]->chunk;
        if (!(chunk->front) && neighbor && neighbor->isAccessible) {
            chunk->front = _chunkList[y][z + 1][x]->chunk;
            chunk->neighbors++;
            chunk->front->back = chunk;
            chunk->front->neighbors++;
        }
    }

    if (y > 0) { //bottom
        neighbor = _chunkList[y - 1][z][x]->chunk;
        if (!(chunk->bottom) && neighbor && neighbor->isAccessible) {
            chunk->bottom = neighbor;
            chunk->neighbors++;
            chunk->bottom->top = chunk;
            chunk->bottom->neighbors++;
        }
    }

    if (y < csGridWidth - 1) { //top
        neighbor = _chunkList[y + 1][z][x]->chunk;
        if (!(chunk->top) && neighbor && neighbor->isAccessible) {
            chunk->top = neighbor;
            chunk->neighbors++;
            chunk->top->bottom = chunk;
            chunk->top->neighbors++;
        }
    }


}

void ChunkManager::drawChunkLines(glm::mat4 &VP, const f64v3& position) {
    Chunk* chunk;
    glm::vec4 color;
    for (i32 i = 0; i < csGridSize; i++) {
        chunk = _allChunkSlots[i].chunk;
        if (chunk) {
            //    if (chunk->drawWater){
            switch (chunk->_state) {
            case ChunkStates::DRAW:
                color = glm::vec4(0.0, 0.0, 1.0, 1.0);
                break;
            case ChunkStates::MESH:
                color = glm::vec4(0.0, 1.0, 0.0, 1.0);
                break;
            case ChunkStates::WATERMESH:
                color = glm::vec4(0.0, 1.0, 1.0, 1.0);
                break;
            default:
                chunk->drawWater = 0;
                color = glm::vec4(0.0, 0.0, 0.0, 1.0);
                break;
            }

            if (chunk->_state != ChunkStates::INACTIVE) DrawWireBox(chunk->position.x, chunk->position.y, chunk->position.z, CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, 2, position, VP, color);
            //    }
        }
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

void ChunkManager::shiftX(i32 dir) {
    Chunk* chunk;

    if (dir == 1) { //Positive x
        //We are now one more chunk further in +x
        cornerPosition.x += CHUNK_WIDTH;
        _hx++;

        //Shift the 2d heightmap
        shiftHeightMap(2);

        //Iterate through a y,z slice
        for (i32 y = 0; y < csGridWidth; y++) {
            for (i32 z = 0; z < csGridWidth; z++) {

                //Append a new ChunkSlot on the +x end of the 3D deque.
                //We can reuse the memory of the ChunkSlot on the -x side, since
                //we are popping it off anyways
                _chunkList[y][z].push_back(_chunkList[y][z][0]);
                // Pop off the - x side ChunkSlot, since we are moving right
                _chunkList[y][z].pop_front();
                //Free the chunk, or put it on a thread waiting list
                freeChunk(_chunkList[y][z].back()->chunk);

                //Initialize the new ChunkSlot
                _chunkList[y][z].back()->Initialize(cornerPosition + glm::ivec3((csGridWidth - 1), y, z) * CHUNK_WIDTH, NULL, _hz + z, _hx + (csGridWidth - 1), _faceMap[z][csGridWidth - 1]);
            }
        }

    } else { //Negative x
        cornerPosition.x -= CHUNK_WIDTH;
        _hx--;

        shiftHeightMap(4);

        //Iterate through a y,z slice
        for (i32 y = 0; y < csGridWidth; y++) {
            for (i32 z = 0; z < csGridWidth; z++) {

                //Append a new ChunkSlot on the -x end of the 3D deque.
                //We can reuse the memory of the ChunkSlot on the +x side, since
                //we are popping it off anyways
                _chunkList[y][z].push_front(_chunkList[y][z].back());
                //Pop off the +x chunk
                _chunkList[y][z].pop_back();

                //Free the chunk, or put it on a thread waiting list
                freeChunk(_chunkList[y][z][0]->chunk);

                //Initialize the new ChunkSlot
                _chunkList[y][z][0]->Initialize(cornerPosition + glm::ivec3(0, y, z) * CHUNK_WIDTH, NULL, _hz + z, _hx, _faceMap[z][0]);
            }
        }

    }
}

void ChunkManager::shiftY(i32 dir) {
    Chunk* chunk;

    if (dir == 1) {
        cornerPosition.y += CHUNK_WIDTH;

        //Add a 2D deque (slice) to top of 3D deque
        _chunkList.push_back(deque <deque <ChunkSlot *> >());

        //Swap with the bottom deque that we are about to free, so we dont
        //Have to call .resize
        _chunkList.back().swap(_chunkList[0]);

        //Iterate through x,z slice
        for (i32 z = 0; z < csGridWidth; z++) {
            for (i32 x = 0; x < csGridWidth; x++) {

                //Free the chunk if its not null
                freeChunk(_chunkList.back()[z][x]->chunk);

                _chunkList.back()[z][x]->Initialize(cornerPosition + glm::ivec3(x, (csGridWidth - 1), z) * CHUNK_WIDTH, NULL, _hz + z, _hx + x, _faceMap[z][x]);
            }
        }

        //Pop off the bottom y 2D deque (slice)
        _chunkList.pop_front();

    } else {
        cornerPosition.y -= CHUNK_WIDTH;

        //Add a 2D deque (slice) to bottom of 3D deque
        _chunkList.push_front(deque <deque <ChunkSlot *> >());

        //Swap with the top deque that we are about to free, so we dont
        //Have to call .resize
        _chunkList[0].swap(_chunkList.back());

        for (i32 z = 0; z < csGridWidth; z++) {
            for (i32 x = 0; x < csGridWidth; x++) {

                freeChunk(_chunkList[0][z][x]->chunk);

                _chunkList[0][z][x]->Initialize(cornerPosition + glm::ivec3(x, 0, z) * CHUNK_WIDTH, NULL, _hz + z, _hx + x, _faceMap[z][x]);
            }
        }
        //Pop off the top y 2D deque (slice)
        _chunkList.pop_back();
    }
}

void ChunkManager::shiftZ(i32 dir) {
    Chunk* chunk;

    if (dir == 1) {
        cornerPosition.z += CHUNK_WIDTH;
        _hz++;

        shiftHeightMap(3);

        for (i32 y = 0; y < csGridWidth; y++) {
            //Create a new empty row in the +z direction, and 
            //swap its memory with the now unused -z one.
            _chunkList[y].push_back(deque <ChunkSlot *>());
            _chunkList[y].back().swap(_chunkList[y][0]);

            for (i32 x = 0; x < csGridWidth; x++) {
                //Free the chunk in this chunk slot
                freeChunk(_chunkList[y].back()[x]->chunk);
                //Initialize this chunkSlot with null chunk
                _chunkList[y].back()[x]->Initialize(cornerPosition + glm::ivec3(x, y, (csGridWidth - 1)) * CHUNK_WIDTH, NULL, _hz + (csGridWidth - 1), _hx + x, _faceMap[csGridWidth - 1][x]);
            }
            //Pop off the -z row
            _chunkList[y].pop_front();
        }

    } else {
        cornerPosition.z -= CHUNK_WIDTH;
        _hz--;

        shiftHeightMap(1);

        for (i32 y = 0; y < csGridWidth; y++) {
            //Create a new empty row in the -z direction, and 
            //swap its memory with the now unused +z one.
            _chunkList[y].push_front(deque <ChunkSlot *>());
            _chunkList[y][0].swap(_chunkList[y].back());

            for (i32 x = 0; x < csGridWidth; x++) {
                //Free the chunk in this chunk slot before we pop it off.

                freeChunk(_chunkList[y][0][x]->chunk);

                //Initialize this chunkSlot with null chunk
                _chunkList[y][0][x]->Initialize(cornerPosition + glm::ivec3(x, y, 0) * CHUNK_WIDTH, nullptr, _hz, _hx + x, _faceMap[0][x]);
            }
            _chunkList[y].pop_back();
        }

    }
}

void ChunkManager::calculateCornerPosition(const f64v3& centerPosition) {
    //Based on the player position, calculate a corner position for the chunks
    cornerPosition.x = (i32)floor((centerPosition.x - CHUNK_WIDTH * csGridWidth / 2.0) / 32.0) * 32;
    cornerPosition.y = (i32)floor((centerPosition.y - CHUNK_WIDTH * csGridWidth / 2.0) / 32.0) * 32;
    cornerPosition.z = (i32)floor((centerPosition.z - CHUNK_WIDTH * csGridWidth / 2.0) / 32.0) * 32;
}

void ChunkManager::updateChunks(const f64v3& position) {
    ChunkSlot *cs;
    Chunk* chunk;
    i32 mx, my, mz;
    double dx, dy, dz;
    double cx, cy, cz;

    mx = (i32)position.x;
    my = (i32)position.y;
    mz = (i32)position.z;
    //ui32 sticks = SDL_GetTicks();

    static ui32 saveTicks = SDL_GetTicks();

    bool save = 0;

    if (SDL_GetTicks() - saveTicks >= 60000 && !generateOnly) { //save once per minute
        save = 1;
        cout << "SAVING\n";
        saveTicks = SDL_GetTicks();
    }
  //  cout << csGridSize << endl;
    for (i32 i = 0; i < csGridSize; i++) { //update distances for all chunks
        cs = &(_allChunkSlots[i]);
        const glm::ivec3 &csPos = cs->position;
        cx = (mx <= csPos.x) ? csPos.x : ((mx > csPos.x + CHUNK_WIDTH) ? (csPos.x + CHUNK_WIDTH) : mx);
        cy = (my <= csPos.y) ? csPos.y : ((my > csPos.y + CHUNK_WIDTH) ? (csPos.y + CHUNK_WIDTH) : my);
        cz = (mz <= csPos.z) ? csPos.z : ((mz > csPos.z + CHUNK_WIDTH) ? (csPos.z + CHUNK_WIDTH) : mz);
        dx = cx - mx;
        dy = cy - my;
        dz = cz - mz;
        //we dont sqrt the distance since sqrt is slow
        cs->distance2 = dx*dx + dy*dy + dz*dz;
        chunk = cs->chunk;

        //update the chunk if it exists
        if (chunk != nullptr) {
            chunk->distance2 = cs->distance2;

            if (cs->distance2 > (graphicsOptions.voxelRenderDistance + 36) * (graphicsOptions.voxelRenderDistance + 36)) { //out of maximum range
                if (!generateOnly && chunk->dirty && chunk->_state > ChunkStates::TREES) {
                    GameManager::chunkIOManager->addToSaveList(cs->chunk);
                }
                freeChunk(chunk);
                cs->chunk = nullptr;
            } else { //inside maximum range

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
        } else { //else grab a new chunk
            if (cs->distance2 <= (graphicsOptions.voxelRenderDistance + 32) * (graphicsOptions.voxelRenderDistance + 32)) {

                chunk = produceChunk();
                chunk->init(cs->position, cs->ipos, cs->jpos, cs->fd);
                chunk->distance2 = cs->distance2;
                cs->chunk = chunk;
                addToLoadList(chunk);
            }
        }
    }
}

void ChunkManager::saveAllChunks() {
    if (generateOnly) return;
    Chunk* chunk;
    for (i32 i = 0; i < csGridSize; i++) { //update distances for all chunks
        chunk = _allChunkSlots[i].chunk;
        if (chunk && chunk->dirty && chunk->_state > ChunkStates::TREES) {
            GameManager::chunkIOManager->addToSaveList(chunk);
        }
    }
}

void ChunkManager::shiftHeightMap(i32 dir) {
    HeightData *tmp;
    FaceData *ftmp;

    if (dir == 1) { // -z
        for (i32 x = 0; x < csGridWidth; x++) {
            tmp = _heightMap[csGridWidth - 1][x];
            ftmp = _faceMap[csGridWidth - 1][x];
            for (i32 z = csGridWidth - 1; z > 0; z--) {
                _heightMap[z][x] = _heightMap[z - 1][x];
                _faceMap[z][x] = _faceMap[z - 1][x];
            }
            _heightMap[0][x] = tmp;
            _faceMap[0][x] = ftmp;
            for (i32 i = 0; i < CHUNK_LAYER; i++) {
                tmp[i].height = UNLOADED_HEIGHT;
            }
            ftmp->ipos = _faceMap[1][x]->ipos - 1;

            //check for face change
            if (ftmp->ipos < 0) {
                ftmp->ipos += (planet->radius * 2) / CHUNK_WIDTH;
                ftmp->face = FaceNeighbors[_faceMap[1][x]->face][(1 + _faceMap[1][x]->rotation) % 4];
                ftmp->rotation = _faceMap[1][x]->rotation + FaceTransitions[_faceMap[1][x]->face][ftmp->face];
                if (ftmp->rotation < 0) {
                    ftmp->rotation += 4;
                } else {
                    ftmp->rotation %= 4;
                }
            } else {
                ftmp->face = _faceMap[1][x]->face;
                ftmp->rotation = _faceMap[1][x]->rotation;
            }
        }
    } else if (dir == 2) { // +x
        for (i32 z = 0; z < csGridWidth; z++) {
            tmp = _heightMap[z][0];
            ftmp = _faceMap[z][0];
            for (i32 x = 0; x < csGridWidth - 1; x++) {
                _heightMap[z][x] = _heightMap[z][x + 1];
                _faceMap[z][x] = _faceMap[z][x + 1];
            }
            _heightMap[z][csGridWidth - 1] = tmp;
            _faceMap[z][csGridWidth - 1] = ftmp;
            for (i32 i = 0; i < CHUNK_LAYER; i++) {
                tmp[i].height = UNLOADED_HEIGHT;
            }
            ftmp->jpos = _faceMap[z][csGridWidth - 2]->jpos + 1;

            //check for face change
            if (ftmp->jpos >= (planet->radius * 2) / CHUNK_WIDTH) {
                ftmp->jpos -= (planet->radius * 2) / CHUNK_WIDTH;
                ftmp->face = FaceNeighbors[_faceMap[z][csGridWidth - 2]->face][_faceMap[z][csGridWidth - 2]->rotation];
                ftmp->rotation = _faceMap[z][csGridWidth - 2]->rotation + FaceTransitions[_faceMap[z][csGridWidth - 2]->face][ftmp->face];
                if (ftmp->rotation < 0) {
                    ftmp->rotation += 4;
                } else {
                    ftmp->rotation %= 4;
                }
            } else {
                ftmp->face = _faceMap[z][csGridWidth - 2]->face;
                ftmp->rotation = _faceMap[z][csGridWidth - 2]->rotation;
            }
        }
    } else if (dir == 3) { // +z
        for (i32 x = 0; x < csGridWidth; x++) {
            tmp = _heightMap[0][x];
            ftmp = _faceMap[0][x];
            for (i32 z = 0; z < csGridWidth - 1; z++) {
                _heightMap[z][x] = _heightMap[z + 1][x];
                _faceMap[z][x] = _faceMap[z + 1][x];
            }
            _heightMap[csGridWidth - 1][x] = tmp;
            _faceMap[csGridWidth - 1][x] = ftmp;

            for (i32 i = 0; i < CHUNK_LAYER; i++) {
                tmp[i].height = UNLOADED_HEIGHT; //tells our game later that it needs to load these heights
                //        faceMap[i][j] = -1;
            }
            ftmp->ipos = _faceMap[csGridWidth - 2][x]->ipos + 1;

            //check for face change
            if (ftmp->ipos >= (planet->radius * 2) / CHUNK_WIDTH) {
                ftmp->ipos -= (planet->radius * 2) / CHUNK_WIDTH;
                ftmp->face = FaceNeighbors[_faceMap[csGridWidth - 2][x]->face][(3 + _faceMap[csGridWidth - 2][x]->rotation) % 4];
                ftmp->rotation = _faceMap[csGridWidth - 2][x]->rotation + FaceTransitions[_faceMap[csGridWidth - 2][x]->face][ftmp->face];
                if (ftmp->rotation < 0) {
                    ftmp->rotation += 4;
                } else {
                    ftmp->rotation %= 4;
                }
            } else {
                ftmp->face = _faceMap[csGridWidth - 2][x]->face;
                ftmp->rotation = _faceMap[csGridWidth - 2][x]->rotation;
            }
        }
    } else if (dir == 4) { // -x
        for (i32 z = 0; z < csGridWidth; z++) {
            tmp = _heightMap[z][csGridWidth - 1];
            ftmp = _faceMap[z][csGridWidth - 1];
            for (i32 x = csGridWidth - 1; x > 0; x--) {
                _heightMap[z][x] = _heightMap[z][x - 1];
                _faceMap[z][x] = _faceMap[z][x - 1];
            }
            _heightMap[z][0] = tmp;
            _faceMap[z][0] = ftmp;
            for (i32 i = 0; i < CHUNK_LAYER; i++) {
                tmp[i].height = UNLOADED_HEIGHT;
            }
            ftmp->jpos = _faceMap[z][1]->jpos - 1;
            //check for face change
            if (ftmp->jpos < 0) {
                ftmp->jpos += (planet->radius * 2) / CHUNK_WIDTH;
                ftmp->face = FaceNeighbors[_faceMap[z][1]->face][(2 + _faceMap[z][1]->rotation) % 4];
                ftmp->rotation = _faceMap[z][1]->rotation + FaceTransitions[_faceMap[z][1]->face][ftmp->face];
                if (ftmp->rotation < 0) {
                    ftmp->rotation += 4;
                } else {
                    ftmp->rotation %= 4;
                }
            } else {
                ftmp->face = _faceMap[z][1]->face;
                ftmp->rotation = _faceMap[z][1]->rotation;
            }
        }
    }
}

void ChunkManager::prepareHeightMap(HeightData heightData[CHUNK_LAYER], i32 startX, i32 startZ, i32 width, i32 height) {
    i32 minNearHeight; //to determine if we should remove surface blocks
    i32 heightDiffThreshold = 3;
    i32 tmp;
    i32 maph;
    Biome *biome;
    i32 sandDepth, snowDepth;
    for (i32 i = startZ; i < startZ + height; i++) {
        for (i32 j = startX; j < startX + width; j++) {

            //*************Calculate if it is too steep ***********************
            maph = heightData[i*CHUNK_WIDTH + j].height;
            biome = heightData[i*CHUNK_WIDTH + j].biome;
            sandDepth = heightData[i*CHUNK_WIDTH + j].sandDepth;
            snowDepth = heightData[i*CHUNK_WIDTH + j].snowDepth;

            minNearHeight = maph;
            if (j > 0) { //Could sentinalize this in the future
                minNearHeight = heightData[i*CHUNK_WIDTH + j - 1].height;
            } else {
                minNearHeight = maph + (maph - heightData[i*CHUNK_WIDTH + j + 1].height); //else use opposite side but negating the difference
            }
            if (j < CHUNK_WIDTH - 1) {
                tmp = heightData[i*CHUNK_WIDTH + j + 1].height;
            } else {
                tmp = maph + (maph - heightData[i*CHUNK_WIDTH + j - 1].height);
            }
            if (tmp < minNearHeight) minNearHeight = tmp;
            if (i > 0) {
                tmp = heightData[(i - 1)*CHUNK_WIDTH + j].height;
            } else {
                tmp = maph + (maph - heightData[(i + 1)*CHUNK_WIDTH + j].height);
            }
            if (tmp < minNearHeight) minNearHeight = tmp;
            if (i < CHUNK_WIDTH - 1) {
                tmp = heightData[(i + 1)*CHUNK_WIDTH + j].height;
            } else {
                tmp = maph + (maph - heightData[(i - 1)*CHUNK_WIDTH + j].height);
            }
            if (tmp < minNearHeight) minNearHeight = tmp;

            if (maph - minNearHeight >= heightDiffThreshold && heightData[i*CHUNK_WIDTH + j].biome->looseSoilDepth) {
                heightData[i*CHUNK_WIDTH + j].flags |= TOOSTEEP;
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
            heightData[i*CHUNK_WIDTH + j].surfaceBlock = surfaceBlock;

            //**************END SURFACE BLOCK CALCULATION******************
        }
    }
}

i32 ChunkManager::getClosestChunks(f64v3 &coords, Chunk* *chunks) {
    i32 xDir, yDir, zDir;
    Chunk* chunk;

    f64v3 relativePos = coords - f64v3(cornerPosition);

    //Get the chunk coordinates (assume its always positive)
    i32 x = (i32)relativePos.x / CHUNK_WIDTH;
    i32 y = (i32)relativePos.y / CHUNK_WIDTH;
    i32 z = (i32)relativePos.z / CHUNK_WIDTH;

    //Determines if were closer to positive or negative chunk
    xDir = (relativePos.x - x > 0.5f) ? 1 : -1;
    yDir = (relativePos.y - y > 0.5f) ? 1 : -1;
    zDir = (relativePos.z - z > 0.5f) ? 1 : -1;

    //clear the memory for the chunk pointer array
    chunks[0] = chunks[1] = chunks[2] = chunks[3] = chunks[4] = chunks[5] = chunks[6] = chunks[7] = nullptr;

    //If the 8 nnearby exist and are accessible then set them in the array. NOTE: Perhaps order matters? 
    if (x > 0 && y > 0 && z > 0 && y < csGridWidth - 1 && x < csGridWidth - 1 && z < csGridWidth - 1) {
        chunk = _chunkList[y][z][x]->chunk;
        if (chunk && chunk->isAccessible) chunks[0] = chunk;
        chunk = _chunkList[y][z][x + xDir]->chunk;
        if (chunk && chunk->isAccessible) chunks[1] = chunk;
        chunk = _chunkList[y][z + zDir][x]->chunk;
        if (chunk && chunk->isAccessible) chunks[2] = chunk;
        chunk = _chunkList[y][z + zDir][x + xDir]->chunk;
        if (chunk && chunk->isAccessible) chunks[3] = chunk;
        chunk = _chunkList[y + yDir][z][x]->chunk;
        if (chunk && chunk->isAccessible) chunks[4] = chunk;
        chunk = _chunkList[y + yDir][z][x + xDir]->chunk;
        if (chunk && chunk->isAccessible) chunks[5] = chunk;
        chunk = _chunkList[y + yDir][z + zDir][x]->chunk;
        if (chunk && chunk->isAccessible) chunks[6] = chunk;
        chunk = _chunkList[y + yDir][z + zDir][x + xDir]->chunk;
        if (chunk && chunk->isAccessible) chunks[7] = chunk;
        return 1;
    }
    return 0;
}

void ChunkManager::clearChunkFromLists(Chunk* chunk) {
    i32 dindex = chunk->drawIndex;
    i32 uindex = chunk->updateIndex;
    vector<Chunk*> *sp;

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

    if (chunk->left && chunk->left->right == chunk) {
        chunk->left->right = nullptr;
        chunk->left->neighbors--;
    }
    if (chunk->right && chunk->right->left == chunk) {
        chunk->right->left = nullptr;
        chunk->right->neighbors--;
    }
    if (chunk->top && chunk->top->bottom == chunk) {
        chunk->top->bottom = nullptr;
        chunk->top->neighbors--;
    }
    if (chunk->bottom && chunk->bottom->top == chunk) {
        chunk->bottom->top = nullptr;
        chunk->bottom->neighbors--;
    }
    if (chunk->front && chunk->front->back == chunk) {
        chunk->front->back = nullptr;
        chunk->front->neighbors--;
    }
    if (chunk->back && chunk->back->front == chunk) {
        chunk->back->front = nullptr;
        chunk->back->neighbors--;
    }
    chunk->neighbors = 0;
    chunk->left = chunk->right = chunk->top = chunk->bottom = chunk->back = chunk->front = nullptr;
}

void ChunkManager::clearChunk(Chunk* chunk) {

    clearChunkFromLists(chunk);
    chunk->clearBuffers();
   
    chunk->freeWaiting = false;
    chunk->clear();

    chunk->inSaveThread = 0;
    chunk->inLoadThread = 0;
}

void ChunkManager::recursiveSortChunks(vector<Chunk*> &v, i32 start, i32 size, i32 type) {
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
    if (chunkPosition.x < 0 || chunkPosition.y < 0 || chunkPosition.z < 0 ||
        chunkPosition.x >= _chunkList.size() || chunkPosition.y >= _chunkList.size() || chunkPosition.z >= _chunkList.size()) {
        *chunk = NULL;
        return;
    }

    *chunk = _chunkList[chunkPosition.y][chunkPosition.z][chunkPosition.x]->chunk;

    if (chunk) {
        //reuse chunkPosition variable to get the position relative to owner chunk
        chunkPosition = relativePosition - chunkPosition * CHUNK_WIDTH;

        blockIndex = chunkPosition.y * CHUNK_LAYER + chunkPosition.z * CHUNK_WIDTH + chunkPosition.x;
    }
}

void ChunkManager::remeshAllChunks() {
    Chunk* chunk;
    for (int i = 0; i < csGridSize; i++) {
        chunk = _allChunkSlots[i].chunk;
        if (chunk) {
            chunk->changeState(ChunkStates::MESH);
        }
    }
}

void ChunkManager::recursiveSortSetupList(vector<Chunk*>& v, i32 start, i32 size, i32 type) {
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

    i32 x, y, z;
    Chunk* ch;

    x = (ppos.x - cornerPosition.x) / CHUNK_WIDTH;
    y = (ppos.y - cornerPosition.y) / CHUNK_WIDTH;
    z = (ppos.z - cornerPosition.z) / CHUNK_WIDTH;

    if (frameCounter == 10 || x != _poccx || y != _poccy || z != _poccz) {
        _poccx = x;
        _poccy = y;
        _poccz = z;

        for (i32 i = 0; i < csGridSize; i++) {
            if (_allChunkSlots[i].chunk) _allChunkSlots[i].chunk->occlude = 1;
        }

        if (x < 0 || y < 0 || z < 0 || x >= csGridWidth || y >= csGridWidth || z >= csGridWidth) return;

        ch = _chunkList[y][z][x]->chunk;
        if ((!ch) || ch->_state == ChunkStates::LOAD) return;

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
    posZ -= cornerPosition.z;
    posX -= cornerPosition.x;
    if (posX >= 0 && posZ >= 0 && posX / CHUNK_WIDTH < (i32)_heightMap[0].size() && posZ / CHUNK_WIDTH < (i32)_heightMap.size()) { //CRASH HERE 11/27/2013
        hd = _heightMap[posZ / CHUNK_WIDTH][posX / CHUNK_WIDTH][(posZ%CHUNK_WIDTH) * CHUNK_WIDTH + posX%CHUNK_WIDTH];
        return 0;
    } else {
        return 1; //fail
    }
}

Chunk* ChunkManager::getChunk(f64v3& position) {

    i32 x, y, z;
    x = (position.x - cornerPosition.x) / CHUNK_WIDTH;
    y = (position.y - cornerPosition.y) / CHUNK_WIDTH;
    z = (position.z - cornerPosition.z) / CHUNK_WIDTH;

    if (x < 0 || y < 0 || z < 0 || x >= csGridWidth || y >= csGridWidth || z >= csGridWidth) {
        return NULL;
    } else {
        return _chunkList[y][z][x]->chunk;
    }
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
                Chunk* c = _chunkList[chunkPos.y][chunkPos.z][chunkPos.x]->chunk;

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
