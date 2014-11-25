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
#include "Camera.h"
#include "CellularAutomataTask.h"
#include "Chunk.h"
#include "ChunkIOManager.h"
#include "ChunkUpdater.h"
#include "FileSystem.h"
#include "FloraGenerator.h"
#include "FloraTask.h"
#include "Frustum.h"
#include "GLEnums.h"
#include "GLProgram.h"
#include "GenerateTask.h"
#include "Mesh.h"
#include "MessageManager.h"
#include "Options.h"
#include "Particles.h"
#include "PhysicsEngine.h"
#include "Planet.h"
#include "RenderTask.h"
#include "Sound.h"
#include "TerrainGenerator.h"
#include "TerrainPatch.h"
#include "ThreadPool.h"
#include "VRayHelper.h"
#include "Vorb.h"
#include "VoxelLightEngine.h"
#include "VoxelPlanetMapper.h"
#include "VoxelRay.h"

const f32 skyR = 135.0f / 255.0f, skyG = 206.0f / 255.0f, skyB = 250.0f / 255.0f;

const i32 CTERRAIN_PATCH_WIDTH = 5;

#define MAX_VOXEL_ARRAYS_TO_CACHE 200
#define NUM_SHORT_VOXEL_ARRAYS 3
#define NUM_BYTE_VOXEL_ARRAYS 1

//TEXTURE TUTORIAL STUFF

//5846 5061
ChunkManager::ChunkManager() : 
    _isStationary(0),
    _cameraVoxelMapData(nullptr),
    _shortFixedSizeArrayRecycler(MAX_VOXEL_ARRAYS_TO_CACHE * NUM_SHORT_VOXEL_ARRAYS),
    _byteFixedSizeArrayRecycler(MAX_VOXEL_ARRAYS_TO_CACHE * NUM_BYTE_VOXEL_ARRAYS) {
    NoChunkFade = 0;
    planet = NULL;
    _poccx = _poccy = _poccz = -1;
    _voxelLightEngine = new VoxelLightEngine();

    // Clear Out The Chunk Diagnostics
    memset(&_chunkDiagnostics, 0, sizeof(ChunkDiagnostics));

    GlobalModelMatrix = glm::mat4(1.0);
}

ChunkManager::~ChunkManager() {
    deleteAllChunks();
    delete _voxelLightEngine;
}

void ChunkManager::initialize(const f64v3& gridPosition, vvoxel::IVoxelMapper* voxelMapper, vvoxel::VoxelMapData* startingMapData, ui32 flags) {

    // Initialize the threadpool for chunk loading
    initializeThreadPool();

    _voxelMapper = voxelMapper;
    // Minerals //TODO(Ben): THIS IS RETARDED
    initializeMinerals();

    // Sun Color Map
    GLubyte sbuffer[64][3];

    glBindTexture(GL_TEXTURE_2D, GameManager::planet->sunColorMapTexture.ID);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, sbuffer);
    for (i32 i = 0; i < 64; i++) {
        sunColor[i][0] = (i32)sbuffer[i][2]; //converts bgr to rgb
        sunColor[i][1] = (i32)sbuffer[i][1];
        sunColor[i][2] = (i32)sbuffer[i][0];
    }

    // IO thread
    GameManager::chunkIOManager->beginThread();

    // Initialize grid
    csGridWidth = 1 + (graphicsOptions.voxelRenderDistance / 32) * 2;
    cout << "GRID WIDTH: " << csGridWidth << endl;
    _csGridSize = csGridWidth * csGridWidth * csGridWidth;
    _chunkSlotMap.reserve(_csGridSize);

    // Set initial capacity of circular buffers
    _setupList.set_capacity(_csGridSize * 2);
    _meshList.set_capacity(_csGridSize * 2);
    _loadList.set_capacity(_csGridSize * 2);
    _generateList.set_capacity(_csGridSize * 2);

    // Reserve capacity for chunkslots. If this capacity is not enough, we will get a 
    // crash. We use * 4 just in case. It should be plenty
    _chunkSlots[0].reserve(_csGridSize * 4);

    // Allocate the camera voxel map data so we can tell where on the grid the camera exists
    _cameraVoxelMapData = _voxelMapper->getNewVoxelMapData(startingMapData);

    // Get the chunk position
    i32v3 chunkPosition;
    chunkPosition.x = fastFloor(gridPosition.x / (float)CHUNK_WIDTH);
    chunkPosition.y = fastFloor(gridPosition.y / (float)CHUNK_WIDTH);
    chunkPosition.z = fastFloor(gridPosition.z / (float)CHUNK_WIDTH);

    // Make the first chunk
    makeChunkAt(chunkPosition, startingMapData);
}

bool sortChunksAscending(const Chunk* a, const Chunk* b) {
    return a->distance2 < b->distance2;
}

bool sortChunksDescending(const Chunk* a, const Chunk* b) {
    return a->distance2 > b->distance2;
}

void ChunkManager::update(const Camera* camera) {

    timeBeginPeriod(1);

    globalMultiplePreciseTimer.setDesiredSamples(10);
    globalMultiplePreciseTimer.start("Update");

    const f64v3& position = camera->getPosition();

    static i32 k = 0;

    //Grid position is used to determine the _cameraVoxelMapData
    i32v3 chunkPosition = getChunkPosition(position);

    i32v2 gridPosition(chunkPosition.x, chunkPosition.z);
    static i32v2 oldGridPosition = gridPosition;

    if (gridPosition != oldGridPosition) {
        _voxelMapper->offsetPosition(_cameraVoxelMapData, i32v2(gridPosition.y - oldGridPosition.y, gridPosition.x - oldGridPosition.x));
    }
    oldGridPosition = gridPosition;

    if (getChunk(chunkPosition) == nullptr) {
        makeChunkAt(chunkPosition, _cameraVoxelMapData);
    }

    sonarDt += 0.003f*physSpeedFactor;
    if (sonarDt > 1.0f) sonarDt = 0.0f;

    globalMultiplePreciseTimer.start("Update Chunks");

    updateChunks(camera);

    globalMultiplePreciseTimer.start("Update Load List");
    updateLoadList(4);

    globalMultiplePreciseTimer.start("Sort");

    if (k >= 8 || (k >= 4 && physSpeedFactor >= 2.0)) {
        std::sort(_setupList.begin(), _setupList.end(), sortChunksDescending);
        std::sort(_meshList.begin(), _meshList.end(), sortChunksDescending);
        std::sort(_loadList.begin(), _loadList.end(), sortChunksDescending);
        k = 0;
    }
    k++;

    globalMultiplePreciseTimer.start("Loaded Chunks");
    updateLoadedChunks(4);

    globalMultiplePreciseTimer.start("Trees To Place List");
    updateTreesToPlace(3);
    globalMultiplePreciseTimer.start("Mesh List");
    updateMeshList(4);
    globalMultiplePreciseTimer.start("Generate List");
    updateGenerateList(4);
    globalMultiplePreciseTimer.start("Setup List");
    updateSetupList(4);

    //This doesn't function correctly
    //caveOcclusion(position);

    globalMultiplePreciseTimer.start("Thread Waiting");
    Chunk* ch;

    for (size_t i = 0; i < _freeWaitingChunks.size();) {
        ch = _freeWaitingChunks[i];
        if (ch->inSaveThread == false && ch->inLoadThread == false && 
            !ch->lastOwnerTask && !ch->_chunkListPtr) {
            freeChunk(_freeWaitingChunks[i]);
            _freeWaitingChunks[i] = _freeWaitingChunks.back();
            _freeWaitingChunks.pop_back();
        } else {
            i++;
        }
    }

    globalMultiplePreciseTimer.start("Finished Tasks");
    processFinishedTasks();
    //change the parameter to true to print out the timings
    globalMultiplePreciseTimer.end(false);

    timeEndPeriod(1);
}

void ChunkManager::getClosestChunks(f64v3 &coords, Chunk** chunks) {
#define GETCHUNK(y, z, x) it = _chunkSlotMap.find(chPos + i32v3(x, y, z)); chunk = (it == _chunkSlotMap.end() ? nullptr : it->second->chunk);

    i32 xDir, yDir, zDir;
    Chunk* chunk;

    std::unordered_map<i32v3, ChunkSlot*>::iterator it;
    vector <ChunkSlot>& chunkSlots = _chunkSlots[0];

    //Get the chunk coordinates (assume its always positive)
    i32v3 chPos = getChunkPosition(coords);

    //Determines if were closer to positive or negative chunk
    xDir = (coords.x - chPos.x > 0.5f) ? 1 : -1;
    yDir = (coords.y - chPos.y > 0.5f) ? 1 : -1;
    zDir = (coords.z - chPos.z > 0.5f) ? 1 : -1;

    //clear the memory for the chunk pointer array
    chunks[0] = chunks[1] = chunks[2] = chunks[3] = chunks[4] = chunks[5] = chunks[6] = chunks[7] = nullptr;

    //If the 8 nearby exist and are accessible then set them in the array. NOTE: Perhaps order matters? 

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
}

i32 ChunkManager::getBlockFromDir(f64v3& dir, f64v3& pos) {
#define MAX_RANGE 200.0
    static const PredBlockID predBlock = [](const i32& id) {
        return id && (id < LOWWATER || id > FULLWATER);
    };
    return VRayHelper::getQuery(pos, f32v3(dir), MAX_RANGE, this, predBlock).id;
}

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

    i32v3 chPos = getChunkPosition(position);

    auto it = _chunkSlotMap.find(chPos);
    if (it == _chunkSlotMap.end()) return nullptr;
    return it->second->chunk;

}

Chunk* ChunkManager::getChunk(const i32v3& chunkPos) {
    auto it = _chunkSlotMap.find(chunkPos);
    if (it == _chunkSlotMap.end()) return nullptr;
    return it->second->chunk;
}

const Chunk* ChunkManager::getChunk(const i32v3& chunkPos) const {
    auto it = _chunkSlotMap.find(chunkPos);
    if (it == _chunkSlotMap.end()) return nullptr;
    return it->second->chunk;
}

ChunkGridData* ChunkManager::getChunkGridData(const i32v2& gridPos) {
    auto it = _chunkGridDataMap.find(gridPos);
    if (it == _chunkGridDataMap.end()) return nullptr;
    return it->second;
}

void ChunkManager::destroy() {
   
    // Clear the chunk IO thread
    GameManager::chunkIOManager->clear();

    // Destroy the thread pool
    _threadPool.destroy();

    _setupList.clear();
    _generateList.clear();
    _meshList.clear();
    _loadList.clear();

    for (size_t i = 0; i < Chunk::possibleMinerals.size(); i++) {
        delete Chunk::possibleMinerals[i];
    }
    Chunk::possibleMinerals.clear();

    for (i32 i = 0; i < _chunkSlots[0].size(); i++) {
        freeChunk(_chunkSlots[0][i].chunk);
    }

    for (auto it = _chunkGridDataMap.begin(); it != _chunkGridDataMap.end(); it++) {
        delete it->second;
    }
    _chunkGridDataMap.clear();

    GameManager::physicsEngine->clearAll();

    for (size_t i = 0; i < _freeWaitingChunks.size(); i++) { //kill the residual waiting threads too
        _freeWaitingChunks[i]->inSaveThread = nullptr;
        freeChunk(_freeWaitingChunks[i]);
    }
    std::vector<Chunk*>().swap(_freeWaitingChunks);

    deleteAllChunks();

    vector<ChunkSlot>().swap(_chunkSlots[0]);

    std::vector<RenderTask*>().swap(_freeRenderTasks);
    std::vector<GenerateTask*>().swap(_freeGenerateTasks);

    _shortFixedSizeArrayRecycler.destroy();
    _byteFixedSizeArrayRecycler.destroy();
}

void ChunkManager::saveAllChunks() {

    Chunk* chunk;
    for (i32 i = 0; i < _chunkSlots[0].size(); i++) { //update distances for all chunks
        chunk = _chunkSlots[0][i].chunk;
        if (chunk && chunk->dirty && chunk->_state > ChunkStates::TREES) {
            GameManager::chunkIOManager->addToSaveList(chunk);
        }
    }
}

void ChunkManager::getBlockAndChunk(const i32v3& blockPosition, Chunk** chunk, int& blockIndex) {

    //Get the chunk coordinates
    i32v3 chunkPosition = getChunkPosition(blockPosition);

    // Get the chunk if it exists
    *chunk = getChunk(chunkPosition);
    if (*chunk == nullptr) {
        return;
    }

    // Get the voxel offset
    i32v3 blockOffset = blockPosition - chunkPosition * CHUNK_WIDTH;

    // Calculate the voxel index
    blockIndex = blockOffset.y * CHUNK_LAYER + blockOffset.z * CHUNK_WIDTH + blockOffset.x;
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

    //   openglManager.debugRenderer->drawCube(
    //       f32v3(start + end) * 0.5f + f32v3(cornerPosition) + f32v3(0.5f), f32v3(size) + f32v3(0.4f),
    //       f32v4(1.0f, 0.0f, 0.0f, 0.3f), 1.0f
    //       );

    return q;
}

void ChunkManager::initializeThreadPool() {
    // Check the hardware concurrency
    size_t hc = thread::hardware_concurrency();
    // Remove two threads for the render thread and main thread
    if (hc > 1) hc--;
    if (hc > 1) hc--;

    // Initialize the threadpool with hc threads
    _threadPool.init(hc);
    // Give some time for the threads to spin up
    SDL_Delay(100);
}

void ChunkManager::processFinishedTasks() {

    #define MAX_TASKS 100

    // Stores tasks for bulk deque
    vcore::IThreadPoolTask* taskBuffer[MAX_TASKS];

    size_t numTasks = _threadPool.getFinishedTasks(taskBuffer, MAX_TASKS);

    vcore::IThreadPoolTask* task;

    for (size_t i = 0; i < numTasks; i++) {
        task = taskBuffer[i];

        // Postprocessing based on task type
        switch (task->getTaskId()) {
            case RENDER_TASK_ID:
                processFinishedRenderTask(static_cast<RenderTask*>(task));
                if (_freeRenderTasks.size() < MAX_CACHED_TASKS) {
                    // Store the render task so we don't have to call new
                    _freeRenderTasks.push_back(static_cast<RenderTask*>(task));
                } else {
                    delete task;
                }
                break;
            case GENERATE_TASK_ID:
                processFinishedGenerateTask(static_cast<GenerateTask*>(task));
                if (_freeGenerateTasks.size() < MAX_CACHED_TASKS) {
                   // Store the generate task so we don't have to call new
                  _freeGenerateTasks.push_back(static_cast<GenerateTask*>(task));
                } else {
                    delete task;
                }
                break;
            case FLORA_TASK_ID:
                processFinishedFloraTask(static_cast<FloraTask*>(task));
                break;
            default:
                pError("Unknown thread pool Task! ID = " + to_string(task->getTaskId()));
                delete task;
                break;
        }
    }
}

void ChunkManager::processFinishedGenerateTask(GenerateTask* task) {
    Chunk *ch = task->chunk;
    if (task == ch->lastOwnerTask) ch->lastOwnerTask = nullptr;
    ch->isAccessible = true;

    if (!(ch->freeWaiting)) {

        setupNeighbors(ch);

        //check to see if the top chunk has light that should end up in this chunk
        _voxelLightEngine->checkTopForSunlight(ch);

        if (ch->treesToLoad.size() || ch->plantsToLoad.size()) {
            ch->_state = ChunkStates::TREES;
            addToSetupList(ch);
        } else {
            ch->_state = ChunkStates::MESH;
            addToMeshList(ch);
        }
    }
}

void ChunkManager::processFinishedRenderTask(RenderTask* task) {
    Chunk* chunk = task->chunk;
    if (task == chunk->lastOwnerTask) chunk->lastOwnerTask = nullptr;
    // Don't do anything if the chunk should be freed
    if (chunk->freeWaiting) return;
    ChunkMeshData *cmd = task->chunkMeshData;

    // Remove the chunk if it is waiting to be freed
    if (chunk->freeWaiting) {
        if (chunk->mesh != NULL) {
            // Chunk no longer needs the mesh handle
            chunk->mesh = NULL;
            // Set the properties to defaults that the mesh is deleted
            *cmd = ChunkMeshData(chunk);
            // Send the message
            GameManager::messageManager->enqueue(ThreadId::UPDATE,
                                                    Message(MessageID::CHUNK_MESH,
                                                    (void *)cmd));
        } else {
            delete cmd;
        }
        return;
    }

    // Add the chunk mesh to the message
    cmd->chunkMesh = chunk->mesh;

    // Check if we need to allocate a new chunk mesh or orphan the current chunk mesh so that it can be freed in openglManager
    switch (cmd->type) {
        case RenderTaskType::DEFAULT:
            if (cmd->waterVertices.empty() && cmd->transVertices.empty() && cmd->vertices.empty() && cmd->cutoutVertices.empty()) {
                chunk->mesh = nullptr;
            } else if (chunk->mesh == nullptr) {
                chunk->mesh = new ChunkMesh(chunk);
                cmd->chunkMesh = chunk->mesh;
            }
            break;
        case RenderTaskType::LIQUID:
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

        GameManager::messageManager->enqueue(ThreadId::UPDATE,
                                                Message(MessageID::CHUNK_MESH,
                                                (void *)cmd));
    }

    if (chunk->_chunkListPtr == nullptr) chunk->_state = ChunkStates::DRAW;
    
}

void ChunkManager::processFinishedFloraTask(FloraTask* task) {
    Chunk* chunk = task->chunk;
    GeneratedTreeNodes* nodes;
    if (task == chunk->lastOwnerTask) chunk->lastOwnerTask = nullptr;
    if (task->isSuccessful) {
        nodes = task->generatedTreeNodes;
        if (nodes->lnodes.size() || nodes->wnodes.size()) {
            _treesToPlace.push_back(nodes);
        } else {
            chunk->_state = ChunkStates::MESH;
            addToMeshList(chunk);
        }
        delete task;
    } else {
        // If the task wasn't successful, add it back to the task queue so it can try again.
        task->setIsFinished(false);
        chunk->lastOwnerTask = task;
        _threadPool.addTask(task);
    }
}

//add the loaded chunks to the setup list
void ChunkManager::updateLoadedChunks(ui32 maxTicks) {

    ui32 startTicks = SDL_GetTicks();
    Chunk* ch;
    TerrainGenerator* generator = GameManager::terrainGenerator;
    //IO load chunks
    while (GameManager::chunkIOManager->finishedLoadChunks.try_dequeue(ch)) {

        ch->inLoadThread = 0;
        
        // Don't do anything if the chunk should be freed
        if (ch->freeWaiting) continue;

        //If the heightmap has not been generated, generate it.
        ChunkGridData* chunkGridData = ch->chunkGridData;
        
        if (chunkGridData->heightData[0].height == UNLOADED_HEIGHT) {
            generator->setVoxelMapping(chunkGridData->voxelMapData, planet->radius, 1.0);
            generator->GenerateHeightMap(chunkGridData->heightData, chunkGridData->voxelMapData->ipos * CHUNK_WIDTH, chunkGridData->voxelMapData->jpos * CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, 1, 0);
            generator->postProcessHeightmap(chunkGridData->heightData);
        }

        if (ch->loadStatus == 1) { //it is not saved. Generate!
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
        
        if (SDL_GetTicks() - startTicks > maxTicks) break;
    }
}

void ChunkManager::makeChunkAt(const i32v3& chunkPosition, const vvoxel::VoxelMapData* relativeMapData, const i32v2& ijOffset /* = i32v2(0) */) {

    // Get the voxel grid position
    i32v2 gridPos(chunkPosition.x, chunkPosition.z);

    // Check and see if the grid data is already allocated here
    ChunkGridData* chunkGridData = getChunkGridData(gridPos);
    if (chunkGridData == nullptr) {
        // If its not allocated, make a new one with a new voxelMapData
        chunkGridData = new ChunkGridData(_voxelMapper->getNewRelativeData(relativeMapData, ijOffset));
        _chunkGridDataMap[gridPos] = chunkGridData;
    } else {
        chunkGridData->refCount++;
    }

    // Add a new chunkslot for the chunk
    _chunkSlots[0].emplace_back(chunkPosition * CHUNK_WIDTH, nullptr, chunkGridData);

    // Make and initialize a chunk
    Chunk* chunk = produceChunk();
    chunk->init(_chunkSlots[0].back().position, &_chunkSlots[0].back());

    // Give the chunk to the chunkSlot
    _chunkSlots[0].back().chunk = chunk;

    // Mark the chunk for loading
    addToLoadList(chunk);

    // Add the chunkSlot to the hashmap keyed on the chunk Position
    _chunkSlotMap[chunk->chunkPosition] = &_chunkSlots[0].back();

    // Connect to any neighbors
    _chunkSlots[0].back().detectNeighbors(_chunkSlotMap);
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

//traverses the back of the load list, popping of entries and giving them to threads to be loaded
void ChunkManager::updateLoadList(ui32 maxTicks) {

    Chunk* chunk;
    vector<Chunk* > chunksToLoad;

    ui32 sticks = SDL_GetTicks();

    while (!_loadList.empty()) {
        chunk = _loadList.back();

        _loadList.pop_back();
        chunk->clearChunkListPtr();
     
        // Check if the chunk is waiting to be freed
        if (chunk->freeWaiting) continue;

        chunk->isAccessible = false;

        chunksToLoad.push_back(chunk);

        if (SDL_GetTicks() - sticks >= maxTicks) {
            break;
        }
    }

    if (chunksToLoad.size()) GameManager::chunkIOManager->addToLoadList(chunksToLoad);
    chunksToLoad.clear();
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

        // Check if the chunk is waiting to be freed
        if (chunk->freeWaiting) {
            // Remove from the setup list
            _setupList[i] = _setupList.back();
            _setupList.pop_back();
            chunk->clearChunkListPtr();
            continue;
        }

        switch (state) {
        case ChunkStates::TREES:
            if (chunk->numNeighbors == 6) {
                FloraTask* floraTask = new FloraTask;
                floraTask->init(chunk);
                chunk->lastOwnerTask = floraTask;
                _threadPool.addTask(floraTask);
                // Remove from the setup list
                _setupList[i] = _setupList.back();
                _setupList.pop_back();
            }
            break;
        default: // chunks that should not be here should be removed
            cout << "ERROR: Chunk with state " << (int)state << " in setup list.\n";
            break;
        }

        if (SDL_GetTicks() - startTicks > maxTicks) break;
    }

    return i;
}

i32 ChunkManager::updateMeshList(ui32 maxTicks) {

    ui32 startTicks = SDL_GetTicks();
    ChunkStates state;
    Chunk* chunk;

    RenderTask *newRenderTask;

    for (i32 i = _meshList.size() - 1; i >= 0; i--) {
        state = _meshList[i]->_state;
        chunk = _meshList[i];

        // If it is waiting to be freed, don't do anything with it
        if (chunk->freeWaiting) {
            // Remove from the mesh list
            _meshList[i] = _meshList.back();
            _meshList.pop_back();
            chunk->clearChunkListPtr();
            continue;
        }

        if (chunk->numNeighbors == 6 && chunk->owner->inFrustum) {     
            
            //TODO: BEN, Need to make sure chunk->numBlocks is always correct
            if (chunk->numBlocks) { 

                chunk->occlude = 0;

                // Get a render task
                if (_freeRenderTasks.size()) {
                    newRenderTask = _freeRenderTasks.back();
                    _freeRenderTasks.pop_back();
                } else {
                    newRenderTask = new RenderTask;
                }

                if (chunk->_state == ChunkStates::MESH) {
                    newRenderTask->init(chunk, RenderTaskType::DEFAULT);
                } else {
                    newRenderTask->init(chunk, RenderTaskType::LIQUID);
                }

                chunk->lastOwnerTask = newRenderTask;
                _threadPool.addTask(newRenderTask);

                // Remove from the mesh list
                _meshList[i] = _meshList.back();
                _meshList.pop_back();
                chunk->clearChunkListPtr();

                chunk->_state = ChunkStates::DRAW;
                
            } else {
                chunk->clearBuffers();
                // Remove from the mesh list
                _meshList[i] = _meshList.back();
                _meshList.pop_back();
                chunk->clearChunkListPtr();

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
    i32 ip, jp;
    GenerateTask* generateTask;

    while (_generateList.size()) {
        chunk = _generateList.front();

        _generateList.pop_front();
        chunk->clearChunkListPtr();

        // If it is waiting to be freed, dont do anything with it
        if (chunk->freeWaiting) continue;

        chunk->isAccessible = false;      

        // Get a generate task
        if (_freeGenerateTasks.size()) {
            generateTask = _freeGenerateTasks.back();
            _freeGenerateTasks.pop_back();
        } else {
            generateTask = new GenerateTask;
        }

        // Init the containers
        chunk->_blockIDContainer.init(vvoxel::VoxelStorageState::FLAT_ARRAY);
        chunk->_lampLightContainer.init(vvoxel::VoxelStorageState::FLAT_ARRAY);
        chunk->_sunlightContainer.init(vvoxel::VoxelStorageState::FLAT_ARRAY);
        chunk->_tertiaryDataContainer.init(vvoxel::VoxelStorageState::FLAT_ARRAY);

        // Initialize the task
        generateTask->init(chunk, new LoadData(chunk->chunkGridData->heightData, GameManager::terrainGenerator));
        chunk->lastOwnerTask = generateTask;
        // Add the task
        _threadPool.addTask(generateTask);

        if (SDL_GetTicks() - startTicks > maxTicks) break;
    }
    return 0;
}

void ChunkManager::updateTreesToPlace(ui32 maxTicks) {
    ui32 startTicks = SDL_GetTicks();
    Chunk* startChunk;

    for (int i = _treesToPlace.size() - 1; i >= 0; i--) {
        // Check for timer end condition
        if (SDL_GetTicks() - startTicks > maxTicks) break;

        GeneratedTreeNodes* nodes = _treesToPlace[i];

        if (nodes->numFrames <= 0) {
            // Check to see if initial chunk is unloaded
            startChunk = getChunk(nodes->startChunkGridPos);
            if (startChunk == nullptr) {
                delete nodes;
                _treesToPlace[i] = _treesToPlace.back();
                _treesToPlace.pop_back();
                continue;
            }
            // Check to see if all the chunks we need are available
            bool allChunksLoaded = true;
            for (auto& it : nodes->allChunkPositions) {
                Chunk* chunk = getChunk(it);
                if (chunk == nullptr || chunk->isAccessible == false) {
                    allChunksLoaded = false;
                    break;
                }
            }
            // Check to see if we can now place the nodes
            if (allChunksLoaded) {
                placeTreeNodes(nodes);
                delete nodes;
                _treesToPlace[i] = _treesToPlace.back();
                _treesToPlace.pop_back();
                // Update startChunk state 
                startChunk->_state = ChunkStates::MESH;
                addToMeshList(startChunk);
            } else {
                // We should wait FRAMES_BEFORE_ATTEMPT frames before retrying
                nodes->numFrames = GeneratedTreeNodes::FRAMES_BEFORE_ATTEMPT;
            }
        } else {
            nodes->numFrames--;
        }
    }
}

void ChunkManager::placeTreeNodes(GeneratedTreeNodes* nodes) {
    // Decompress all chunks to arrays for efficiency
    for (auto& it : nodes->allChunkPositions) {
        Chunk* chunk = getChunk(it);
        if (chunk->_blockIDContainer.getState() == vvoxel::VoxelStorageState::INTERVAL_TREE) {
            chunk->_blockIDContainer.changeState(vvoxel::VoxelStorageState::FLAT_ARRAY, chunk->_dataLock);
        }
        if (chunk->_sunlightContainer.getState() == vvoxel::VoxelStorageState::INTERVAL_TREE) {
            chunk->_sunlightContainer.changeState(vvoxel::VoxelStorageState::FLAT_ARRAY, chunk->_dataLock);
        }
    }

    int blockIndex;
    Chunk* owner;
    Chunk* lockedChunk = nullptr;
    const i32v3& startPos = nodes->startChunkGridPos;

    for (auto& node : nodes->wnodes) { //wood nodes
        blockIndex = node.blockIndex;
       
        owner = getChunk(startPos + FloraTask::getChunkOffset(node.chunkOffset));
        // Lock the chunk
        if (lockedChunk) lockedChunk->unlock();
        lockedChunk = owner;
        lockedChunk->lock();

        ChunkUpdater::placeBlockNoUpdate(owner, blockIndex, node.blockType);

        // TODO(Ben): Use a smother property for block instead of this hard coded garbage
        if (blockIndex >= CHUNK_LAYER) {
            if (owner->getBlockID(blockIndex - CHUNK_LAYER) == (ui16)Blocks::DIRTGRASS) owner->setBlockID(blockIndex - CHUNK_LAYER, (ui16)Blocks::DIRT); //replace grass with dirt
        } else if (owner->bottom && owner->bottom->isAccessible) {
            // Lock the chunk
            if (lockedChunk) lockedChunk->unlock();
            lockedChunk = owner->bottom;
            lockedChunk->lock();

            if (owner->bottom->getBlockID(blockIndex + CHUNK_SIZE - CHUNK_LAYER) == (ui16)Blocks::DIRTGRASS) owner->bottom->setBlockID(blockIndex + CHUNK_SIZE - CHUNK_LAYER, (ui16)Blocks::DIRT);
        }
    }

    for (auto& node : nodes->lnodes) { //leaf nodes
        blockIndex = node.blockIndex;
        owner = getChunk(startPos + FloraTask::getChunkOffset(node.chunkOffset));
        // Lock the chunk
        if (lockedChunk) lockedChunk->unlock();
        lockedChunk = owner;
        lockedChunk->lock();

        int blockID = owner->getBlockData(blockIndex);

        if (blockID == (ui16)Blocks::NONE) {
            ChunkUpdater::placeBlockNoUpdate(owner, blockIndex, node.blockType);
        }
    }

    // Dont forget to unlock
    if (lockedChunk) lockedChunk->unlock();
}

void ChunkManager::setupNeighbors(Chunk* chunk) {
    i32v3 chPos;
    chPos.x = fastFloor(chunk->gridPosition.x / (float)CHUNK_WIDTH);
    chPos.y = fastFloor(chunk->gridPosition.y / (float)CHUNK_WIDTH);
    chPos.z = fastFloor(chunk->gridPosition.z / (float)CHUNK_WIDTH);
    ChunkSlot* neighborSlot;
    ChunkSlot* owner = chunk->owner;

    //left
    if (owner->left && !chunk->left) {
        neighborSlot = owner->left;
        if (neighborSlot->chunk && neighborSlot->chunk->isAccessible) {
            chunk->left = neighborSlot->chunk;
            chunk->numNeighbors++;
           
            chunk->left->right = chunk;
            chunk->left->numNeighbors++;
        }
    }
    
    //right
    if (owner->right && !chunk->right) {
        neighborSlot = owner->right;
        if (neighborSlot->chunk && neighborSlot->chunk->isAccessible) {
            chunk->right = neighborSlot->chunk;
            chunk->numNeighbors++;
          
            chunk->right->left = chunk;
            chunk->right->numNeighbors++;
        }
    }
    
    //back
    if (owner->back && !chunk->back) {
        neighborSlot = owner->back;
        if (neighborSlot->chunk && neighborSlot->chunk->isAccessible) {
            chunk->back = neighborSlot->chunk;
            chunk->numNeighbors++;
            
            chunk->back->front = chunk;
            chunk->back->numNeighbors++;
        }
    }
    
    //front
    if (owner->front && !chunk->front) {
        neighborSlot = owner->front;
        if (neighborSlot->chunk && neighborSlot->chunk->isAccessible) {
            chunk->front = neighborSlot->chunk;
            chunk->numNeighbors++;
           
            chunk->front->back = chunk;
            chunk->front->numNeighbors++;
        }
    }
    
    //bottom
    if (owner->bottom && !chunk->bottom) {
        neighborSlot = owner->bottom;
        if (neighborSlot->chunk && neighborSlot->chunk->isAccessible) {
            chunk->bottom = neighborSlot->chunk;
            chunk->numNeighbors++;

            chunk->bottom->top = chunk;
            chunk->bottom->numNeighbors++;
        }
    }

    //top
    if (owner->top && !chunk->top) {
        neighborSlot = owner->top;
        if (neighborSlot->chunk && neighborSlot->chunk->isAccessible) {
            chunk->top = neighborSlot->chunk;
            chunk->numNeighbors++;
           
            chunk->top->bottom = chunk;
            chunk->top->numNeighbors++;
        }
    }
}

void ChunkManager::updateCaPhysics() {
    static unsigned int frameCounter = 0;
    static unsigned int powderCounter = 0;
    static unsigned int waterCounter = 0;

    bool updatePowders = false;
    bool updateWater = false;

    if (powderCounter >= 4 || (powderCounter == 2 && physSpeedFactor >= 2.0)) {
        if (isWaterUpdating) updatePowders = true;
        powderCounter = 0;
    }

    if (waterCounter >= 2 || (waterCounter == 1 && physSpeedFactor >= 2.0)) {
        if (isWaterUpdating) updateWater = true;
        waterCounter = 0;
    }

    const std::vector<ChunkSlot>& chunkSlots = _chunkSlots[0];
    CellularAutomataTask* caTask;
    Chunk* chunk;
    if (updateWater && updatePowders) {
        for (int i = 0; i < chunkSlots.size(); i++) {
            chunk = chunkSlots[i].chunk;
            if (chunk && (chunk->hasCaUpdates(0) || chunk->hasCaUpdates(1) || chunk->hasCaUpdates(2))) {
                caTask = new CellularAutomataTask(chunk, CA_FLAG_POWDER | CA_FLAG_LIQUID);
                chunk->lastOwnerTask = caTask;
                _threadPool.addTask(caTask);
            }
        }
    } else if (updateWater) {
        for (int i = 0; i < chunkSlots.size(); i++) {
            chunk = chunkSlots[i].chunk;
            if (chunk && chunk->hasCaUpdates(0)) {
                caTask = new CellularAutomataTask(chunk, CA_FLAG_LIQUID);
                chunk->lastOwnerTask = caTask;
                _threadPool.addTask(caTask);
            }
        }
    } else if (updatePowders) {
        for (int i = 0; i < chunkSlots.size(); i++) {
            chunk = chunkSlots[i].chunk;
            if (chunk && (chunk->hasCaUpdates(1) || chunk->hasCaUpdates(2))) {
                caTask = new CellularAutomataTask(chunk, CA_FLAG_POWDER);
                chunk->lastOwnerTask = caTask;
                _threadPool.addTask(caTask);
            }
        }
    }

    frameCounter++;
    powderCounter++;
    waterCounter++;
    if (frameCounter == 3) frameCounter = 0;
}

void ChunkManager::freeChunk(Chunk* chunk) {
    if (chunk) {
        if (chunk->dirty && chunk->_state > ChunkStates::TREES) {
            GameManager::chunkIOManager->addToSaveList(chunk);
        }
        // Clear any opengl buffers
        chunk->clearBuffers();
        // Lock to prevent data races
        chunk->lock();
        // Sever any connections with neighbor chunks
        chunk->clearNeighbors();
        if (chunk->inSaveThread || chunk->inLoadThread || chunk->_chunkListPtr) {
            // Mark the chunk as waiting to be finished with threads and add to threadWaiting list
            chunk->freeWaiting = true;
            chunk->distance2 = 0; // make its distance 0 so it gets processed first in the lists and gets removed
            chunk->unlock();
            _freeWaitingChunks.push_back(chunk);
        } else {
            // Reduce the ref count since the chunk no longer needs chunkGridData
            chunk->chunkGridData->refCount--;
            // Check to see if we should free the grid data
            if (chunk->chunkGridData->refCount == 0) {
                i32v2 gridPosition(chunk->chunkPosition.x, chunk->chunkPosition.z);
                _chunkGridDataMap.erase(gridPosition);
                delete chunk->chunkGridData;
            }
            // Completely clear the chunk and then recycle it
            
            chunk->clear();
            chunk->unlock();
            recycleChunk(chunk);
        }
    }
}

void ChunkManager::addToSetupList(Chunk* chunk) {
    chunk->addToChunkList(&_setupList);
}

void ChunkManager::addToLoadList(Chunk* chunk) {
    chunk->_state = ChunkStates::LOAD;
    chunk->addToChunkList(&_loadList);
}

void ChunkManager::addToGenerateList(Chunk* chunk) {
    chunk->_state = ChunkStates::GENERATE;
    chunk->addToChunkList(&_generateList);
}

void ChunkManager::addToMeshList(Chunk* chunk) {
    chunk->addToChunkList(&_meshList);
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
    return new Chunk(&_shortFixedSizeArrayRecycler, &_byteFixedSizeArrayRecycler);
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

void ChunkManager::updateChunks(const Camera* camera) {

    ChunkSlot *cs;
    Chunk* chunk;

    i32v3 chPos;
    const f64v3 position = camera->getPosition();
    i32v3 intPosition(position);

    //ui32 sticks = SDL_GetTicks();

    static ui32 saveTicks = SDL_GetTicks();

    bool save = 0;

    #define MS_PER_MINUTE 60000

    if (SDL_GetTicks() - saveTicks >= MS_PER_MINUTE) { //save once per minute
        save = 1;
        cout << "SAVING\n";
        saveTicks = SDL_GetTicks();
    }

    for (i32 i = (i32)_chunkSlots[0].size()-1; i >= 0; i--) { //update distances for all chunks
        cs = &(_chunkSlots[0][i]);

        cs->calculateDistance2(intPosition);
        chunk = cs->chunk;
        if (chunk->_state > ChunkStates::TREES) {
            chunk->updateContainers();
        }

        if (cs->distance2 > (graphicsOptions.voxelRenderDistance + 36) * (graphicsOptions.voxelRenderDistance + 36)) { //out of maximum range
           
            // Only remove it if it isn't needed by its neighbors
            if (!chunk->lastOwnerTask && !chunk->isAdjacentInThread()) {
                if (chunk->dirty && chunk->_state > ChunkStates::TREES) {
                    GameManager::chunkIOManager->addToSaveList(cs->chunk);
                }
                _chunkSlotMap.erase(chunk->chunkPosition);

                freeChunk(chunk);
                cs->chunk = nullptr;

                cs->clearNeighbors();

                _chunkSlots[0][i] = _chunkSlots[0].back();
                _chunkSlots[0].pop_back();
                if (i < _chunkSlots[0].size()) {
                    _chunkSlots[0][i].reconnectToNeighbors();
                    _chunkSlotMap[_chunkSlots[0][i].chunk->chunkPosition] = &(_chunkSlots[0][i]);
                }
            }
        } else { //inside maximum range

            // Check if it is in the view frustum
            cs->inFrustum = camera->sphereInFrustum(f32v3(f64v3(cs->position) + f64v3(CHUNK_WIDTH / 2) - position), 28.0f);

            // See if neighbors need to be added
            if (cs->numNeighbors != 6) {
                updateChunkslotNeighbors(cs, intPosition);
            }

            // Calculate the LOD as a power of two
            int newLOD = (int)(sqrt(chunk->distance2) / graphicsOptions.voxelLODThreshold) + 1;
            //  newLOD = 2;
            if (newLOD > 6) newLOD = 6;
            if (newLOD != chunk->getLevelOfDetail()) {
                chunk->setLevelOfDetail(newLOD);
                chunk->changeState(ChunkStates::MESH);
            }

            if (isWaterUpdating && chunk->mesh != nullptr) ChunkUpdater::randomBlockUpdates(chunk);

            // Check to see if it needs to be added to the mesh list
            if (chunk->_chunkListPtr == nullptr && chunk->lastOwnerTask == false) {
                switch (chunk->_state) {
                case ChunkStates::WATERMESH:
                case ChunkStates::MESH:
                    addToMeshList(chunk);
                    break;
                default:
                    break;
                }
            }

            // save if its been a minute
            if (save && chunk->dirty) {
                GameManager::chunkIOManager->addToSaveList(chunk);  
            }
        }
    }

}

void ChunkManager::updateChunkslotNeighbors(ChunkSlot* cs, const i32v3& cameraPos) {

    if (cs->left == nullptr) {
        tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(-1, 0, 0));
    }
    if (cs->right == nullptr) {
        tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(1, 0, 0));
    }
    if (cs->back == nullptr) {
        tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(0, 0, -1));
    }
    if (cs->front == nullptr) {
        tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(0, 0, 1));
    }
    if (cs->bottom == nullptr) {
        tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(0, -1, 0));
    }
    if (cs->top == nullptr) {
        tryLoadChunkslotNeighbor(cs, cameraPos, i32v3(0, 1, 0));
    }
}

ChunkSlot* ChunkManager::tryLoadChunkslotNeighbor(ChunkSlot* cs, const i32v3& cameraPos, const i32v3& offset) {
    i32v3 newPosition = cs->position + offset * CHUNK_WIDTH;
   
    double dist2 = ChunkSlot::getDistance2(newPosition, cameraPos);
    double dist = sqrt(dist2);
    if (dist2 <= (graphicsOptions.voxelRenderDistance + CHUNK_WIDTH) * (graphicsOptions.voxelRenderDistance + CHUNK_WIDTH)) {

        i32v3 chunkPosition = getChunkPosition(newPosition);

        i32v2 ijOffset(offset.z, offset.x);
        makeChunkAt(chunkPosition, cs->chunkGridData->voxelMapData, ijOffset);

        return nullptr;
        
    }
    return nullptr;
}

void ChunkManager::caveOcclusion(const f64v3& ppos) {
    return; // TODO(Ben): tis broken
    static ui32 frameCounter = 0;
    frameCounter++;

    i32v3 chPos;
    Chunk* ch;

//    chPos.x = (ppos.x - cornerPosition.x) / CHUNK_WIDTH;
//    chPos.y = (ppos.y - cornerPosition.y) / CHUNK_WIDTH;
//    chPos.z = (ppos.z - cornerPosition.z) / CHUNK_WIDTH;

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