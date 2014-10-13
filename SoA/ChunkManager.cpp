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

#include "VoxelPlanetMapper.h"

const f32 skyR = 135.0f / 255.0f, skyG = 206.0f / 255.0f, skyB = 250.0f / 255.0f;

const i32 CTERRAIN_PATCH_WIDTH = 5;

//TEXTURE TUTORIAL STUFF

//5846 5061
ChunkManager::ChunkManager() : _isStationary(0), _cameraVoxelMapData(nullptr) {
    NoChunkFade = 0;
    planet = NULL;
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

    GlobalModelMatrix = glm::mat4(1.0);
}

ChunkManager::~ChunkManager() {
    deleteAllChunks();
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

void ChunkManager::update(const f64v3& position, const f64v3& viewDir) {

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
    Chunk::modifyLock.lock();

    updateChunks(position);

    Chunk::modifyLock.unlock();

    globalMultiplePreciseTimer.start("Update Load List");
    updateLoadList(4);
    //  globalMultiplePreciseTimer.start("CAEngine Update");
    //  GameManager::caEngine->update(*this);

    globalMultiplePreciseTimer.start("Loaded Chunks");
    updateLoadedChunks();

    globalMultiplePreciseTimer.start("Sort");
    //  cout << "BEGIN SORT\n";
    //   fflush(stdout);



    if (k >= 8 || (k >= 4 && physSpeedFactor >= 2.0)) {
        recursiveSortChunkList(_setupList, 0, _setupList.size());
        recursiveSortChunkList(_meshList, 0, _meshList.size());
        recursiveSortChunkList(_loadList, 0, _loadList.size());
        k = 0;
    }
    k++;

    // cout << "END SORT\n";
    // fflush(stdout);
    globalMultiplePreciseTimer.start("Mesh List");
    updateMeshList(4);
    globalMultiplePreciseTimer.start("Generate List");
    updateGenerateList(4);
    globalMultiplePreciseTimer.start("Setup List");
    updateSetupList(4);

    //This doesnt function correctly
    //caveOcclusion(position);
    //  globalMultiplePreciseTimer.start("Physics Engine");
    //Chunk::modifyLock.lock();
    // GameManager::physicsEngine->update(viewDir);
    //Chunk::modifyLock.unlock();

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
        if (!chunk) continue;
        posOffset = f32v3(f64v3(chunk->gridPosition) - position);

        if (((chunk->mesh && chunk->mesh->inFrustum) || SphereInFrustum((float)(posOffset.x + CHUNK_WIDTH / 2), (float)(posOffset.y + CHUNK_WIDTH / 2), (float)(posOffset.z + CHUNK_WIDTH / 2), 28.0f, gridFrustum))) {


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

void ChunkManager::clearAll() {
    // Clear the threadpool
    threadPool.clearJobs();
    while (!(threadPool.isFinished()));

    // Clear finished generating chunks
    std::vector<Chunk*>().swap(taskQueueManager.finishedChunks);

    // Clear finished chunk meshes
    for (size_t i = 0; i < taskQueueManager.finishedChunkMeshes.size(); i++) delete taskQueueManager.finishedChunkMeshes[i];
    std::vector<ChunkMeshData *>().swap(_finishedChunkMeshes);

    // Clear the chunk IO thread
    GameManager::chunkIOManager->clear();

    // Close the threadpool
    threadPool.close();

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

    for (size_t i = 0; i < _threadWaitingChunks.size(); i++) { //kill the residual waiting threads too
        _threadWaitingChunks[i]->clear();
        recycleChunk(_threadWaitingChunks[i]);
    }
    std::vector<Chunk*>().swap(_threadWaitingChunks);

    deleteAllChunks();

    vector<ChunkSlot>().swap(_chunkSlots[0]);
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
    threadPool.initialize(hc);
    // Give some time for the threads to spin up
    SDL_Delay(100);
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

void ChunkManager::uploadFinishedMeshes() {
    Chunk* chunk;
    ChunkMeshData *cmd;

    taskQueueManager.frLock.lock();
    for (size_t i = 0; i < taskQueueManager.finishedChunkMeshes.size(); i++) {
        _finishedChunkMeshes.push_back(taskQueueManager.finishedChunkMeshes[i]);
        assert(taskQueueManager.finishedChunkMeshes[i]->chunk != nullptr);
        taskQueueManager.finishedChunkMeshes[i]->chunk->inFinishedMeshes = 0;
    }
    taskQueueManager.finishedChunkMeshes.clear();
    taskQueueManager.frLock.unlock();

    //use the temp vector so that the threads dont have to wait.
    while (_finishedChunkMeshes.size()) {
        cmd = _finishedChunkMeshes.back();
        _finishedChunkMeshes.pop_back();
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

        if (chunk->_chunkListPtr == nullptr) chunk->_state = ChunkStates::DRAW;
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
    ChunkGridData* chunkGridData;

    ui32 sticks = SDL_GetTicks();
    while (!_loadList.empty()) {
        chunk = _loadList.back();
        _loadList.pop_back();

        chunk->clearChunkListPtr();
     
        chunk->isAccessible = 0;

        chunkGridData = chunk->chunkGridData;

        //If the heightmap has not been generated, generate it.
        if (chunkGridData->heightData[0].height == UNLOADED_HEIGHT) {

            currTerrainGenerator->setVoxelMapping(chunkGridData->voxelMapData, planet->radius, 1.0);

            currTerrainGenerator->GenerateHeightMap(chunkGridData->heightData, chunkGridData->voxelMapData->ipos * CHUNK_WIDTH, chunkGridData->voxelMapData->jpos * CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH, 1, 0);
            currTerrainGenerator->postProcessHeightmap(chunkGridData->heightData);
        }

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

        switch (state) {
        case ChunkStates::TREES:
            if (chunk->numNeighbors == 6) {
                if (chunk->treeTryTicks == 0) { //keep us from continuing to try a tree when it wont ever load
                    if (FloraGenerator::generateFlora(chunk)) {
                        chunk->_state = ChunkStates::MESH;

                        chunk->removeFromChunkList();

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
            chunk->removeFromChunkList();
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

    for (i32 i = _meshList.size() - 1; i >= 0; i--) {
        state = _meshList[i]->_state;
        chunk = _meshList[i];

        if (chunk->numNeighbors == 6 && chunk->owner->inFrustum) {     
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
                    chunk->removeFromChunkList();
                }
            } else {
                chunk->clearBuffers();
                chunk->removeFromChunkList();

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

    while (_generateList.size()) {
        chunk = _generateList.front();
       
        chunk->clearChunkListPtr();

        _generateList.pop_front();

        chunk->isAccessible = 0;

        threadPool.addLoadJob(chunk, new LoadData(chunk->chunkGridData->heightData, currTerrainGenerator));

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

void ChunkManager::clearChunkFromLists(Chunk* chunk) {

    // Clear any opengl buffers
    chunk->clearBuffers();

    // Remove from any chunk list
    if (chunk->_chunkListPtr != nullptr) {
        chunk->removeFromChunkList();
    }

    // Remove from finished chunks queue
    if (chunk->inFinishedChunks) {
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

    // Remove from finished meshes queue
    if (chunk->inFinishedMeshes) {
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

    // Sever any connections with neighbor chunks
    chunk->clearNeighbors();
}

void ChunkManager::freeChunk(Chunk* chunk) {
    if (chunk) {
        if (chunk->dirty && chunk->_state > ChunkStates::TREES) {
            GameManager::chunkIOManager->addToSaveList(chunk);
        }
        // Remove the chunk from any important lists
        clearChunkFromLists(chunk);
        if (chunk->inSaveThread || chunk->inRenderThread || chunk->inLoadThread || chunk->inGenerateThread) {
            // Mark the chunk as waiting to be finished with threads and add to threadWaiting list
            chunk->freeWaiting = true;
            _threadWaitingChunks.push_back(chunk);
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

void ChunkManager::updateChunks(const f64v3& position) {

    ChunkSlot *cs;
    Chunk* chunk;

    i32v3 chPos;
    i32v3 intPosition(position);

    //ui32 sticks = SDL_GetTicks();

    static ui32 saveTicks = SDL_GetTicks();

    bool save = 0;

    if (SDL_GetTicks() - saveTicks >= 60000) { //save once per minute
        save = 1;
        cout << "SAVING\n";
        saveTicks = SDL_GetTicks();
    }

    for (i32 i = (i32)_chunkSlots[0].size()-1; i >= 0; i--) { //update distances for all chunks
        cs = &(_chunkSlots[0][i]);

        cs->calculateDistance2(intPosition);
        chunk = cs->chunk;

        if (cs->distance2 > (graphicsOptions.voxelRenderDistance + 36) * (graphicsOptions.voxelRenderDistance + 36)) { //out of maximum range
           
            if (chunk->dirty && chunk->_state > ChunkStates::TREES) {
                GameManager::chunkIOManager->addToSaveList(cs->chunk);
            }
            _chunkSlotMap.erase(chunk->chunkPosition);

            chunk->clearNeighbors();
            freeChunk(chunk);
            cs->chunk = nullptr;
            
            cs->clearNeighbors();
          
            _chunkSlots[0][i] = _chunkSlots[0].back();
            _chunkSlots[0].pop_back();
            if (i < _chunkSlots[0].size()) {
                _chunkSlots[0][i].reconnectToNeighbors();
                _chunkSlotMap[_chunkSlots[0][i].chunk->chunkPosition] = &(_chunkSlots[0][i]);
            }
        } else { //inside maximum range

            // Check if it is in the view frustum
            cs->inFrustum = SphereInFrustum((float)(cs->position.x + CHUNK_WIDTH / 2 - position.x),
                                            (float)(cs->position.y + CHUNK_WIDTH / 2 - position.y),
                                            (float)(cs->position.z + CHUNK_WIDTH / 2 - position.z), 28.0f, gridFrustum);

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

            // calculate light stuff: THIS CAN LAG THE GAME
            if (chunk->_state > ChunkStates::TREES) {
                if (chunk->sunRemovalList.size()) {
                    VoxelLightEngine::calculateSunlightRemoval(chunk);
                }
                if (chunk->sunExtendList.size()) {
                    VoxelLightEngine::calculateSunlightExtend(chunk);
                }
                VoxelLightEngine::calculateLight(chunk);
            }
            // Check to see if it needs to be added to the mesh list
            if (chunk->_chunkListPtr == nullptr && chunk->inRenderThread == false) {
                switch (chunk->_state) {
                case ChunkStates::WATERMESH:
                case ChunkStates::MESH:
                    addToMeshList(chunk);
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

void ChunkManager::recursiveSortChunkList(boost::circular_buffer<Chunk*>& v, i32 start, i32 size) {
    if (size < 2) return;
    i32 i, j;
    Chunk* pivot, *mid, *last, *tmp;

    pivot = v[start];

    //end recursion when small enough
    if (size == 2) {
        if ((pivot->distance2 - pivot->setupWaitingTime) < (v[start + 1]->distance2 - v[start + 1]->setupWaitingTime)) {
            v[start] = v[start + 1];
            v[start + 1] = pivot;

            v[start]->_chunkListIndex = start;
            v[start + 1]->_chunkListIndex = start + 1;
            
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

     
        mid->_chunkListIndex = start;
        pivot->_chunkListIndex = start + size / 2;
        

        pivot = mid;
        pd = md;
    } else if ((pd > ld && ld > md) || (pd < ld && ld < md)) {
        v[start] = last;

        v[start + size - 1] = pivot;


        last->_chunkListIndex = start;
        pivot->_chunkListIndex = start + size - 1;
        

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

            v[i]->_chunkListIndex = i;
            v[j]->_chunkListIndex = j;
            
            i++;
            j--;
        }
    }

    //swap pivot with rightmost element in left set
    v[start] = v[j];
    v[j] = pivot;

    v[start]->_chunkListIndex = start;
    v[j]->_chunkListIndex = j;
    

    //sort the two subsets excluding the pivot point
    recursiveSortChunkList(v, start, j - start);
    recursiveSortChunkList(v, j + 1, start + size - j - 1);
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