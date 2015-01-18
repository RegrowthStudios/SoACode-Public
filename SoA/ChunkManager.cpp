#include "stdafx.h"
#include "ChunkManager.h"

#include <deque>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <Vorb/graphics/GLEnums.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/Mesh.h>
#include <Vorb/ThreadPool.h>
#include <Vorb/Vorb.h>
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
#include "GenerateTask.h"
#include "MessageManager.h"
#include "Options.h"
#include "Particles.h"
#include "PhysicsEngine.h"
#include "RenderTask.h"
#include "Sound.h"
#include "SphericalTerrainGenerator.h"

#include "SphericalTerrainPatch.h"

#include "VRayHelper.h"
#include "VoxelLightEngine.h"
#include "VoxelPlanetMapper.h"
#include "VoxelRay.h"

const f32 skyR = 135.0f / 255.0f, skyG = 206.0f / 255.0f, skyB = 250.0f / 255.0f;

const i32 CTERRAIN_PATCH_WIDTH = 5;

#define MAX_VOXEL_ARRAYS_TO_CACHE 200
#define NUM_SHORT_VOXEL_ARRAYS 3
#define NUM_BYTE_VOXEL_ARRAYS 1

bool HeightmapGenRpcDispatcher::dispatchHeightmapGen(ChunkGridData* cgd, vvox::VoxelPlanetMapData* mapData) {
    // Check if there is a free generator
    if (!m_generators[counter].inUse) {
        auto& gen = m_generators[counter];
        // Mark the generator as in use
        gen.inUse = true;
        cgd->wasRequestSent = true;
        gen.gridData = cgd;
        gen.rpc.data.f = &gen;
        // Set the data
        gen.startPos.x = mapData->jpos;
        gen.startPos.y = mapData->ipos;
        gen.coordMapping.x = vvox::FaceCoords[mapData->face][mapData->rotation][1];
        gen.coordMapping.y = vvox::FaceCoords[mapData->face][mapData->rotation][2];
        gen.coordMapping.z = vvox::FaceCoords[mapData->face][mapData->rotation][0];
        gen.width = 32;
        gen.step = 1;
        // Invoke generator
        m_generator->invokeRawGen(&gen.rpc);
        // Go to next generator
        counter++;
        if (counter == NUM_GENERATORS) counter = 0;
        return true;
    }
    return false;
}

ChunkManager::ChunkManager(PhysicsEngine* physicsEngine, vvox::IVoxelMapper* voxelMapper,
                           SphericalTerrainGenerator* terrainGenerator,
                           const vvox::VoxelMapData* startingMapData, ChunkIOManager* chunkIo,
                           const f64v3& gridPosition) :
    _isStationary(0),
    _cameraVoxelMapData(nullptr),
    _shortFixedSizeArrayRecycler(MAX_VOXEL_ARRAYS_TO_CACHE * NUM_SHORT_VOXEL_ARRAYS),
    _byteFixedSizeArrayRecycler(MAX_VOXEL_ARRAYS_TO_CACHE * NUM_BYTE_VOXEL_ARRAYS),
    _voxelMapper(voxelMapper),
    m_terrainGenerator(terrainGenerator),
    m_physicsEngine(physicsEngine),
    m_chunkIo(chunkIo) {
   
    m_physicsEngine->setChunkManager(this);

    NoChunkFade = 0;
    planet = NULL;
    _poccx = _poccy = _poccz = -1;
    _voxelLightEngine = new VoxelLightEngine();

    // Clear Out The Chunk Diagnostics
    memset(&_chunkDiagnostics, 0, sizeof(ChunkDiagnostics));

    heightmapGenRpcDispatcher = std::make_unique<HeightmapGenRpcDispatcher>(m_terrainGenerator);

    // Initialize the threadpool for chunk loading
    initializeThreadPool();

    // IO thread
    m_chunkIo->beginThread();

    // Initialize grid
    csGridWidth = 1 + (graphicsOptions.voxelRenderDistance / 32) * 2;
    std::cout << "GRID WIDTH: " << csGridWidth << std::endl;
    _csGridSize = csGridWidth * csGridWidth * csGridWidth;
    m_chunkMap.reserve(_csGridSize);
    m_chunks.reserve(_csGridSize);

    // Set initial capacity of stacks for efficiency
    _setupList.reserve(_csGridSize / 2);
    _meshList.reserve(_csGridSize / 2);
    _loadList.reserve(_csGridSize / 2);

    // Allocate the camera voxel map data so we can tell where on the grid the camera exists
    _cameraVoxelMapData = _voxelMapper->getNewVoxelMapData(startingMapData);

    // Get the chunk position
    i32v3 chunkPosition;
    chunkPosition.x = fastFloor(gridPosition.x / (double)CHUNK_WIDTH);
    chunkPosition.y = fastFloor(gridPosition.y / (double)CHUNK_WIDTH);
    chunkPosition.z = fastFloor(gridPosition.z / (double)CHUNK_WIDTH);

    // Make the first chunk
    makeChunkAt(chunkPosition, startingMapData);
}

ChunkManager::~ChunkManager() {
    deleteAllChunks();
    delete _voxelLightEngine;
}

bool sortChunksAscending(const Chunk* a, const Chunk* b) {
    return a->distance2 < b->distance2;
}

bool sortChunksDescending(const Chunk* a, const Chunk* b) {
    return a->distance2 > b->distance2;
}

void ChunkManager::update(const f64v3& position) {

    timeBeginPeriod(1);

    globalMultiplePreciseTimer.setDesiredSamples(10);
    globalMultiplePreciseTimer.start("Update");

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

    updateChunks(position);

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
   // std::cout << "TASKS " << _threadPool.getFinishedTasksSizeApprox() << std::endl;
    globalMultiplePreciseTimer.start("Loaded Chunks");
    updateLoadedChunks(4);
    globalMultiplePreciseTimer.start("Trees To Place List");
    updateTreesToPlace(3);
    globalMultiplePreciseTimer.start("Mesh List");
    updateMeshList(4);
    globalMultiplePreciseTimer.start("Setup List");
    updateSetupList(4);

    updateGenerateList();

    //This doesn't function correctly
    //caveOcclusion(position);

    globalMultiplePreciseTimer.start("Thread Waiting");
    Chunk* ch;

    for (size_t i = 0; i < _freeWaitingChunks.size();) {
        ch = _freeWaitingChunks[i];
        if (ch->inSaveThread == false && ch->inLoadThread == false && 
            !ch->lastOwnerTask && !ch->_chunkListPtr && ch->chunkDependencies == 0) {
            ch->clearNeighbors();
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

    static int g = 0;
    if (++g == 10) {
        globalAccumulationTimer.printAll(false);
        std::cout << "\n";
        globalAccumulationTimer.clear();
        g = 0;
    }
}

void ChunkManager::getClosestChunks(f64v3 &coords, Chunk** chunks) {
#define GETCHUNK(y, z, x) it = m_chunkMap.find(chPos + i32v3(x, y, z)); chunk = (it == m_chunkMap.end() ? nullptr : it->second);

    i32 xDir, yDir, zDir;
    Chunk* chunk;
    std::unordered_map<i32v3, Chunk*>::iterator it; // Used in macro

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

    auto it = m_chunkMap.find(chPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;

}

Chunk* ChunkManager::getChunk(const i32v3& chunkPos) {
    auto it = m_chunkMap.find(chunkPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;
}

const Chunk* ChunkManager::getChunk(const i32v3& chunkPos) const {
    auto it = m_chunkMap.find(chunkPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;
}

ChunkGridData* ChunkManager::getChunkGridData(const i32v2& gridPos) {
    auto it = _chunkGridDataMap.find(gridPos);
    if (it == _chunkGridDataMap.end()) return nullptr;
    return it->second;
}

void ChunkManager::destroy() {
   
    // Clear the chunk IO thread
    m_chunkIo->clear();

    heightmapGenRpcDispatcher.reset();

    // Destroy the thread pool
    _threadPool.destroy();

    _setupList.clear();
    _meshList.clear();
    _loadList.clear();

    for (size_t i = 0; i < Chunk::possibleMinerals.size(); i++) {
        delete Chunk::possibleMinerals[i];
    }
    Chunk::possibleMinerals.clear();

    for (i32 i = 0; i < m_chunks.size(); i++) {
        freeChunk(m_chunks[i]);
    }

    for (auto it = _chunkGridDataMap.begin(); it != _chunkGridDataMap.end(); it++) {
        delete it->second;
    }
    _chunkGridDataMap.clear();

    for (size_t i = 0; i < _freeWaitingChunks.size(); i++) { //kill the residual waiting threads too
        _freeWaitingChunks[i]->inSaveThread = nullptr;
        freeChunk(_freeWaitingChunks[i]);
    }
    std::vector<Chunk*>().swap(_freeWaitingChunks);

    deleteAllChunks();

    std::vector<Chunk*>().swap(m_chunks);

    std::vector<RenderTask*>().swap(_freeRenderTasks);
    std::vector<GenerateTask*>().swap(_freeGenerateTasks);

    _shortFixedSizeArrayRecycler.destroy();
    _byteFixedSizeArrayRecycler.destroy();
}

void ChunkManager::saveAllChunks() {

    Chunk* chunk;
    for (i32 i = 0; i < m_chunks.size(); i++) { //update distances for all chunks
        chunk = m_chunks[i];
        if (chunk && chunk->dirty && chunk->_state > ChunkStates::TREES) {
            m_chunkIo->addToSaveList(chunk);
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
    for (int i = 0; i < m_chunks.size(); i++) {
        m_chunks[i]->changeState(ChunkStates::MESH);
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
    size_t hc = std::thread::hardware_concurrency();
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
    vcore::IThreadPoolTask<WorkerData>* taskBuffer[MAX_TASKS];

    size_t numTasks = _threadPool.getFinishedTasks(taskBuffer, MAX_TASKS);

    vcore::IThreadPoolTask<WorkerData>* task;
    Chunk* chunk;

    for (size_t i = 0; i < numTasks; i++) {
        task = taskBuffer[i];

        // Post processing based on task type
        switch (task->getTaskId()) {
            case RENDER_TASK_ID:
                chunk = static_cast<RenderTask*>(task)->chunk;
                tryRemoveMeshDependencies(chunk);
                if (task == chunk->lastOwnerTask) chunk->lastOwnerTask = nullptr;
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
            case CA_TASK_ID:
                chunk = static_cast<CellularAutomataTask*>(task)->_chunk;
                if (task == chunk->lastOwnerTask) {
                    chunk->lastOwnerTask = nullptr;
                }
                if (task == chunk->lastOwnerTask) chunk->lastOwnerTask = nullptr;
                if (_freeRenderTasks.size() < MAX_CACHED_TASKS) {
                    // Store the render task so we don't have to call new
                    _freeRenderTasks.push_back(static_cast<CellularAutomataTask*>(task)->renderTask);
                } else {
                    delete static_cast<CellularAutomataTask*>(task)->renderTask;
                }
                _numCaTasks--;
                delete task;
                break;
            default:
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
    //IO load chunks
    while (m_chunkIo->finishedLoadChunks.try_dequeue(ch)) {

        bool canGenerate = true;
        ch->inLoadThread = 0;
        
        // Don't do anything if the chunk should be freed
        if (ch->freeWaiting) continue;

        //If the heightmap has not been generated, generate it.
        ChunkGridData* chunkGridData = ch->chunkGridData;
        
        //TODO(Ben): Beware of race here.
        if (chunkGridData->heightData[0].height == UNLOADED_HEIGHT) {
         //   if (!chunkGridData->wasRequestSent) {
          //      // Keep trying to send it until it succeeds
         //       while (!heightmapGenRpcDispatcher->dispatchHeightmapGen(chunkGridData,
        //            (vvox::VoxelPlanetMapData*)ch->voxelMapData));
        //    }
            for (int i = 0; i < 1024; i++) {
                chunkGridData->heightData[i].height = 0;
                chunkGridData->heightData[i].temperature = 128;
                chunkGridData->heightData[i].rainfall = 128;
                chunkGridData->heightData[i].biome = nullptr;
                chunkGridData->heightData[i].surfaceBlock = STONE;
                chunkGridData->heightData[i].snowDepth = 0;
                chunkGridData->heightData[i].sandDepth = 0;
                chunkGridData->heightData[i].depth = 0;
                chunkGridData->heightData[i].flags = 0;
            }
          //  canGenerate = false;
        }

        // If it is not saved. Generate it!
        if (ch->loadStatus == 1) {
            ch->loadStatus == 0;

            // If we can generate immediately, then do so. Otherwise we wait
            if (canGenerate) {
                addGenerateTask(ch);
            } else {
                addToGenerateList(ch);
            }
        } else {
            ch->_state = ChunkStates::MESH;
            addToMeshList(ch);
            ch->dirty = false;
            ch->isAccessible = true;
        }
        
        if (SDL_GetTicks() - startTicks > maxTicks) break;
    }
}

void ChunkManager::updateGenerateList() {
    Chunk *chunk;
    for (i32 i = m_generateList.size() - 1; i >= 0; i--) {
        chunk = m_generateList[i];

        
    }
}

void ChunkManager::addGenerateTask(Chunk* chunk) {

    GenerateTask* generateTask;
    // Get a generate task
    if (_freeGenerateTasks.size()) {
        generateTask = _freeGenerateTasks.back();
        _freeGenerateTasks.pop_back();
    } else {
        generateTask = new GenerateTask;
    }

    // Init the containers
    chunk->_blockIDContainer.init(vvox::VoxelStorageState::FLAT_ARRAY);
    chunk->_lampLightContainer.init(vvox::VoxelStorageState::FLAT_ARRAY);
    chunk->_sunlightContainer.init(vvox::VoxelStorageState::FLAT_ARRAY);
    chunk->_tertiaryDataContainer.init(vvox::VoxelStorageState::FLAT_ARRAY);

    // Initialize the task
    generateTask->init(chunk, new LoadData(chunk->chunkGridData->heightData));
    chunk->lastOwnerTask = generateTask;
    // Add the task
    _threadPool.addTask(generateTask);
}

void ChunkManager::makeChunkAt(const i32v3& chunkPosition, const vvox::VoxelMapData* relativeMapData, const i32v2& ijOffset /* = i32v2(0) */) {

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

    // Make and initialize a chunk
    Chunk* chunk = produceChunk();
    chunk->init(chunkPosition, chunkGridData);

    m_chunks.push_back(chunk);

    // Mark the chunk for loading
    addToLoadList(chunk);

    // Add the chunkSlot to the hashmap keyed on the chunk Position
    m_chunkMap[chunkPosition] = chunk;

    // Connect to any neighbors
    chunk->detectNeighbors(m_chunkMap);
}

//traverses the back of the load list, popping of entries and giving them to threads to be loaded
void ChunkManager::updateLoadList(ui32 maxTicks) {

    Chunk* chunk;
    std::vector<Chunk* > chunksToLoad;

    ui32 sticks = SDL_GetTicks();

    while (!_loadList.empty()) {
        chunk = _loadList.back();

        _loadList.pop_back();
        chunk->clearChunkListPtr();
     
        // Check if the chunk is waiting to be freed
        if (chunk->freeWaiting) continue;

        chunksToLoad.push_back(chunk);

        if (SDL_GetTicks() - sticks >= maxTicks) {
            break;
        }
    }

    if (chunksToLoad.size()) m_chunkIo->addToLoadList(chunksToLoad);
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
                chunk->clearChunkListPtr();
            }
            break;
        default: // chunks that should not be here should be removed
            std::cout << "ERROR: Chunk with state " << (int)state << " in setup list.\n";
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

        // If it has no solid blocks, dont mesh it
        if (!chunk->numBlocks) {
            // Remove from the mesh list
            _meshList[i] = _meshList.back();
            _meshList.pop_back();
            chunk->clearChunkListPtr();
            chunk->_state = ChunkStates::INACTIVE;
            continue;
        }

        if (chunk->inFrustum && trySetMeshDependencies(chunk)) {     
           
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
        }


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
        if (chunk->_blockIDContainer.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
            chunk->_blockIDContainer.changeState(vvox::VoxelStorageState::FLAT_ARRAY, chunk->_dataLock);
        }
        if (chunk->_sunlightContainer.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
            chunk->_sunlightContainer.changeState(vvox::VoxelStorageState::FLAT_ARRAY, chunk->_dataLock);
        }
    }

    int blockIndex;
    Chunk* owner;
    Chunk* lockedChunk = nullptr;
    const i32v3& startPos = nodes->startChunkGridPos;

    int a = 0;
    for (auto& node : nodes->wnodes) { //wood nodes
        blockIndex = node.blockIndex;
       
        owner = getChunk(startPos + FloraTask::getChunkOffset(node.chunkOffset));
        // Lock the chunk
        vvox::swapLockedChunk(owner, lockedChunk);

        ChunkUpdater::placeBlockNoUpdate(owner, blockIndex, node.blockType);
        // TODO(Ben): Use a smother transform property for block instead of this hard coded garbage
        int blockID = GETBLOCKID(vvox::getBottomBlockData(owner, lockedChunk, blockIndex, blockIndex, owner));
        if (blockID == DIRTGRASS) {
            owner->setBlockData(blockIndex, DIRT);
        }
    }

    for (auto& node : nodes->lnodes) { //leaf nodes
        blockIndex = node.blockIndex;
        owner = getChunk(startPos + FloraTask::getChunkOffset(node.chunkOffset));
        // Lock the chunk
        vvox::swapLockedChunk(owner, lockedChunk);

        int blockID = owner->getBlockData(blockIndex);

        if (blockID == NONE) {
            ChunkUpdater::placeBlockNoUpdate(owner, blockIndex, node.blockType);
        }
    }

    // Dont forget to unlock
    if (lockedChunk) lockedChunk->unlock();
}

void ChunkManager::updateCaPhysics() {

    // TODO(Ben): Semi-fixed timestep
    if (_numCaTasks == 0) {
        std::vector <CaPhysicsType*> typesToUpdate;
        // Check which types need to update
        for (auto& type : CaPhysicsType::typesArray) {
            if (type->update()) {
                typesToUpdate.push_back(type);
            }
        }

        if (typesToUpdate.size()) {
            CellularAutomataTask* caTask;
            Chunk* chunk;
            
            // Loop through all chunk slots and update chunks that have updates
            for (int i = 0; i < m_chunks.size(); i++) {
                chunk = m_chunks[i];
                if (chunk && chunk->numNeighbors == 6 && chunk->hasCaUpdates(typesToUpdate)) {
                    caTask = new CellularAutomataTask(this, m_physicsEngine, chunk, chunk->inFrustum);
                    for (auto& type : typesToUpdate) {
                        caTask->addCaTypeToUpdate(type);
                    }
                    chunk->lastOwnerTask = caTask;
                    _threadPool.addTask(caTask);
                    _numCaTasks++;
                }
            }
        }
    }
}

void ChunkManager::freeChunk(Chunk* chunk) {
    if (chunk) {
        
        if (chunk->dirty && chunk->_state > ChunkStates::TREES) {
            m_chunkIo->addToSaveList(chunk);
        }
        // Clear any opengl buffers
        chunk->clearBuffers();

        if (chunk->inSaveThread || chunk->inLoadThread || chunk->_chunkListPtr || chunk->lastOwnerTask) {
            globalAccumulationTimer.start("FREE B");
            // Mark the chunk as waiting to be finished with threads and add to threadWaiting list
            chunk->freeWaiting = true;
            chunk->distance2 = 0; // make its distance 0 so it gets processed first in the lists and gets removed

            _freeWaitingChunks.push_back(chunk);
            globalAccumulationTimer.stop();
        } else {

            // Reduce the ref count since the chunk no longer needs chunkGridData
            chunk->chunkGridData->refCount--;
            // Check to see if we should free the grid data
            if (chunk->chunkGridData->refCount == 0) {
                globalAccumulationTimer.start("FREE C");
                i32v2 gridPosition(chunk->chunkPosition.x, chunk->chunkPosition.z);
                _chunkGridDataMap.erase(gridPosition);
                globalAccumulationTimer.start("FREE D");
                delete chunk->chunkGridData;
                globalAccumulationTimer.stop();
            }
            // Completely clear the chunk and then recycle it
            
            // Destroy the mesh
            chunk->clearBuffers();

            chunk->clear();
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

void ChunkManager::addToMeshList(Chunk* chunk) {
    if (!chunk->queuedForMesh) {
        chunk->addToChunkList(&_meshList);
        chunk->queuedForMesh = true;
    }
}

void ChunkManager::addToGenerateList(Chunk* chunk) {
    chunk->_state = ChunkStates::GENERATE;
    chunk->addToChunkList(&m_generateList);
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
    return new Chunk(&_shortFixedSizeArrayRecycler, &_byteFixedSizeArrayRecycler, CaPhysicsType::getNumCaTypes());
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

void ChunkManager::updateChunks(const f64v3& position) {

    Chunk* chunk;

    i32v3 chPos;
    i32v3 intPosition(position);

    //ui32 sticks = SDL_GetTicks();

    static ui32 saveTicks = SDL_GetTicks();

    bool save = 0;

    #define MS_PER_MINUTE 60000

    if (SDL_GetTicks() - saveTicks >= MS_PER_MINUTE) { //save once per minute
        save = 1;
        std::cout << "SAVING\n";
        saveTicks = SDL_GetTicks();
    }

    for (i32 i = (i32)m_chunks.size() - 1; i >= 0; i--) { //update distances for all chunks
        chunk = m_chunks[i];

        chunk->calculateDistance2(intPosition);

        globalAccumulationTimer.start("UC");
        if (chunk->_state > ChunkStates::TREES) {
            chunk->updateContainers();
        }
        globalAccumulationTimer.stop();
        if (chunk->distance2 > (graphicsOptions.voxelRenderDistance + 36) * (graphicsOptions.voxelRenderDistance + 36)) { //out of maximum range
           
            // Only remove it if it isn't needed by its neighbors
            if (!chunk->lastOwnerTask && !chunk->chunkDependencies) {
                if (chunk->dirty && chunk->_state > ChunkStates::TREES) {
                    m_chunkIo->addToSaveList(chunk);
                }
                m_chunkMap.erase(chunk->chunkPosition);

                freeChunk(chunk);

                m_chunks[i] = m_chunks.back();
                m_chunks.pop_back();
               
                globalAccumulationTimer.stop();
            }
        } else { //inside maximum range

            // Check if it is in the view frustum
            //cs->inFrustum = camera->sphereInFrustum(f32v3(f64v3(cs->position) + f64v3(CHUNK_WIDTH / 2) - position), 28.0f);
            chunk->inFrustum = true; // TODO(Ben): Pass in a frustum?

            // See if neighbors need to be added
            if (chunk->numNeighbors != 6) {
                globalAccumulationTimer.start("CSN");
                updateChunkNeighbors(chunk, intPosition);
                globalAccumulationTimer.stop();
            }
            globalAccumulationTimer.start("REST");
            // Calculate the LOD as a power of two
            int newLOD = (int)(sqrt(chunk->distance2) / graphicsOptions.voxelLODThreshold) + 1;
            //  newLOD = 2;
            if (newLOD > 6) newLOD = 6;
            if (newLOD != chunk->getLevelOfDetail()) {
                chunk->setLevelOfDetail(newLOD);
                chunk->changeState(ChunkStates::MESH);
            }

            if (isWaterUpdating && chunk->mesh != nullptr) ChunkUpdater::randomBlockUpdates(this, m_physicsEngine, chunk);

            // Check to see if it needs to be added to the mesh list
            if (chunk->_chunkListPtr == nullptr && chunk->lastOwnerTask == false) {
                switch (chunk->_state) {
                case ChunkStates::WATERMESH:
                case ChunkStates::MESH:
                    addToMeshList(chunk);
                    globalAccumulationTimer.stop();
                    break;
                default:
                    globalAccumulationTimer.stop();
                    break;
                }
            }

            // save if its been a minute
            if (save && chunk->dirty) {
                m_chunkIo->addToSaveList(chunk);
            }
            globalAccumulationTimer.stop();
        }
    }

}

void ChunkManager::updateChunkNeighbors(Chunk* chunk, const i32v3& cameraPos) {

    if (chunk->left == nullptr) {
        tryLoadChunkNeighbor(chunk, cameraPos, i32v3(-1, 0, 0));
    }
    if (chunk->right == nullptr) {
        tryLoadChunkNeighbor(chunk, cameraPos, i32v3(1, 0, 0));
    }
    if (chunk->back == nullptr) {
        tryLoadChunkNeighbor(chunk, cameraPos, i32v3(0, 0, -1));
    }
    if (chunk->front == nullptr) {
        tryLoadChunkNeighbor(chunk, cameraPos, i32v3(0, 0, 1));
    }
    if (chunk->bottom == nullptr) {
        tryLoadChunkNeighbor(chunk, cameraPos, i32v3(0, -1, 0));
    }
    if (chunk->top == nullptr) {
        tryLoadChunkNeighbor(chunk, cameraPos, i32v3(0, 1, 0));
    }
}

void ChunkManager::tryLoadChunkNeighbor(Chunk* chunk, const i32v3& cameraPos, const i32v3& offset) {
    i32v3 newPosition = chunk->gridPosition + offset * CHUNK_WIDTH;
   
    double dist2 = Chunk::getDistance2(newPosition, cameraPos);
    if (dist2 <= (graphicsOptions.voxelRenderDistance + CHUNK_WIDTH) * (graphicsOptions.voxelRenderDistance + CHUNK_WIDTH)) {

        i32v3 chunkPosition = getChunkPosition(newPosition);

        i32v2 ijOffset(offset.z, offset.x);
        makeChunkAt(chunkPosition, chunk->chunkGridData->voxelMapData, ijOffset);
    }
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

    //    for (i32 i = 0; i < _chunkSlots[0].size(); i++) {
     //       if (_chunkSlots[0][i].chunk) _chunkSlots[0][i].chunk->occlude = 1;
    //    }

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

void ChunkManager::printOwnerList(Chunk* chunk) {
    if (chunk->_chunkListPtr) {
        if (chunk->_chunkListPtr == &_freeList) {
            std::cout << "freeList ";
        } else if (chunk->_chunkListPtr == &_setupList) {
            std::cout << "setupList ";
        } else if (chunk->_chunkListPtr == &_meshList) {
            std::cout << "meshList ";
        } else if (chunk->_chunkListPtr == &_loadList) {
            std::cout << "loadList ";
        } else if (chunk->_chunkListPtr == &m_generateList) {
            std::cout << "generateList ";
        }
    } else {
        std::cout << "NO LIST ";
    }
}

bool ChunkManager::trySetMeshDependencies(Chunk* chunk) {
    // If this chunk is still in a mesh thread, don't re-add dependencies
    if (chunk->meshJobCounter) {
        chunk->meshJobCounter++;
        return true;
    }
    if (chunk->numNeighbors != 6) return false;

    Chunk* nc;

    // Neighbors
    if (!chunk->left->isAccessible || !chunk->right->isAccessible ||
        !chunk->front->isAccessible || !chunk->back->isAccessible ||
        !chunk->top->isAccessible || !chunk->bottom->isAccessible) return false;
   
    // Left Side
    if (!chunk->left->back || !chunk->left->back->isAccessible) return false;
    if (!chunk->left->front || !chunk->left->front->isAccessible) return false;
    nc = chunk->left->top;
    if (!nc || !nc->isAccessible) return false;
    if (!nc->back || !nc->back->isAccessible) return false;
    if (!nc->front || !nc->front->isAccessible) return false;
    nc = chunk->left->bottom;
    if (!nc || !nc->isAccessible) return false;
    if (!nc->back || !nc->back->isAccessible) return false;
    if (!nc->front || !nc->front->isAccessible) return false;
    
    // Right side
    if (!chunk->right->back || !chunk->right->back->isAccessible) return false;
    if (!chunk->right->front || !chunk->right->front->isAccessible) return false;
    nc = chunk->right->top;
    if (!nc || !nc->isAccessible) return false;
    if (!nc->back || !nc->back->isAccessible) return false;
    if (!nc->front || !nc->front->isAccessible) return false;
    nc = chunk->right->bottom;
    if (!nc || !nc->isAccessible) return false;
    if (!nc->back || !nc->back->isAccessible) return false;
    if (!nc->front || !nc->front->isAccessible) return false;

    // Front
    if (!chunk->front->top || !chunk->front->top->isAccessible) return false;
    if (!chunk->front->bottom || !chunk->front->bottom->isAccessible) return false;

    // Back
    if (!chunk->back->top || !chunk->back->top->isAccessible) return false;
    if (!chunk->back->bottom || !chunk->back->bottom->isAccessible) return false;

    // If we get here, we can set dependencies

    // Neighbors
    chunk->left->addDependency();
    chunk->right->addDependency();
    chunk->front->addDependency();
    chunk->back->addDependency();
    chunk->top->addDependency();
    chunk->bottom->addDependency();

    // Left Side
    chunk->left->back->addDependency();
    chunk->left->front->addDependency();
    nc = chunk->left->top;
    nc->addDependency();
    nc->back->addDependency();
    nc->front->addDependency();
    nc = chunk->left->bottom;
    nc->addDependency();
    nc->back->addDependency();
    nc->front->addDependency();

    // Right side
    chunk->right->back->addDependency();
    chunk->right->front->addDependency();
    nc = chunk->right->top;
    nc->addDependency();
    nc->back->addDependency();
    nc->front->addDependency();
    nc = chunk->right->bottom;
    nc->addDependency();
    nc->back->addDependency();
    nc->front->addDependency();

    // Front
    chunk->front->top->addDependency();
    chunk->front->bottom->addDependency();

    // Back
    chunk->back->top->addDependency();
    chunk->back->bottom->addDependency();

    chunk->meshJobCounter++;
    return true;
}

void ChunkManager::tryRemoveMeshDependencies(Chunk* chunk) {
    chunk->meshJobCounter--;
    // If this chunk is still in a mesh thread, don't remove dependencies
    if (chunk->meshJobCounter) return;

    Chunk* nc;
    // Neighbors
    chunk->left->removeDependency();
    chunk->right->removeDependency();
    chunk->front->removeDependency();
    chunk->back->removeDependency();
    chunk->top->removeDependency();
    chunk->bottom->removeDependency();

    // Left Side
    chunk->left->back->removeDependency();
    chunk->left->front->removeDependency();
    nc = chunk->left->top;
    nc->removeDependency();
    nc->back->removeDependency();
    nc->front->removeDependency();
    nc = chunk->left->bottom;
    nc->removeDependency();
    nc->back->removeDependency();
    nc->front->removeDependency();

    // Right side
    chunk->right->back->removeDependency();
    chunk->right->front->removeDependency();
    nc = chunk->right->top;
    nc->removeDependency();
    nc->back->removeDependency();
    nc->front->removeDependency();
    nc = chunk->right->bottom;
    nc->removeDependency();
    nc->back->removeDependency();
    nc->front->removeDependency();

    // Front
    chunk->front->top->removeDependency();
    chunk->front->bottom->removeDependency();

    // Back
    chunk->back->top->removeDependency();
    chunk->back->bottom->removeDependency();
}
