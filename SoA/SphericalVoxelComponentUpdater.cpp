#include "stdafx.h"
#include "SphericalVoxelComponentUpdater.h"

#include <SDL/SDL_timer.h> // For SDL_GetTicks

#include "ChunkAllocator.h"
#include "ChunkIOManager.h"
#include "ChunkListManager.h"
#include "ChunkRenderer.h"
#include "ChunkUpdater.h"
#include "FloraTask.h"
#include "GameSystem.h"
#include "GenerateTask.h"
#include "NChunk.h"
#include "NChunkGrid.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "PlanetData.h"
#include "RenderTask.h"
#include "SoaOptions.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainGpuGenerator.h"
#include "soaUtils.h"

#include <Vorb/voxel/VoxCommon.h>

void SphericalVoxelComponentUpdater::update(const SoaState* soaState) {
    SpaceSystem* spaceSystem = soaState->spaceSystem.get();
    GameSystem* gameSystem = soaState->gameSystem.get();
    if (spaceSystem->m_sphericalVoxelCT.getComponentListSize() > 1) {

        // TODO(Ben): This is temporary hard coded player stuff.
        auto& playerPosCmp = gameSystem->voxelPosition.getFromEntity(soaState->playerEntity);
        auto& playerFrustumCmp = gameSystem->frustum.getFromEntity(soaState->playerEntity);

        for (auto& it : spaceSystem->m_sphericalVoxelCT) {
            if (it.second.chunkGrid) {
                m_cmp = &it.second;
                updateComponent(playerPosCmp.gridPosition.pos, &playerFrustumCmp.frustum);
            }
        }
    }
}

void SphericalVoxelComponentUpdater::updateComponent(const f64v3& position, const Frustum* frustum) {

    // Always make a chunk at camera location
    i32v3 chunkPosition = VoxelSpaceConversions::voxelToChunk(position);
    if (m_cmp->chunkGrid->getChunk(chunkPosition) == nullptr) {
        ChunkPosition3D cPos;
        cPos.pos = chunkPosition;
        cPos.face = m_cmp->chunkGrid->getFace();
        makeChunkAt(cPos);
    }

    updateChunks(position, frustum);

    updateLoadList(4);

    // TODO(Ben): There are better ways to do this
    if (m_cmp->updateCount >= 8) {
        m_cmp->chunkListManager->sortLists();
        m_cmp->updateCount = 0;
    }

    m_cmp->updateCount++;
    // std::cout << "TASKS " << _threadPool.getFinishedTasksSizeApprox() << std::endl;
    updateLoadedChunks(4);
    updateTreesToPlace(3);
    updateMeshList(4);
    updateSetupList(4);
    updateGenerateList();

    //This doesn't function correctly
    //caveOcclusion(position);

    NChunk* ch;
    std::vector<NChunk*>& freeWaitingChunks = m_cmp->chunkListManager->freeWaitingChunks;
    for (size_t i = 0; i < freeWaitingChunks.size();) {
        ch = freeWaitingChunks[i];
        if (ch->inSaveThread == false && ch->inLoadThread == false &&
            !ch->lastOwnerTask && !ch->_chunkListPtr && ch->chunkDependencies == 0 &&
            !(ch->mesh && ch->mesh->refCount)) {
            NChunk* c = freeWaitingChunks[i];
            freeWaitingChunks[i] = freeWaitingChunks.back();
            freeWaitingChunks.pop_back();
            freeChunk(c);
        } else {
            i++;
        }
    }

    processFinishedTasks();
}

void SphericalVoxelComponentUpdater::updateChunks(const f64v3& position, const Frustum* frustum) {

    i32v3 iPosition(position);
    // TODO(Ben): We are iterating over empty and hidden chunks...
    const std::vector<NChunk*>& chunks = m_cmp->chunkAllocator->getActiveChunks();
    f32 fadeDist2 = FLT_MAX;

    // Reset the counter for number of container compressions
    vvox::clearContainerCompressionsCounter();

    for (int i = (int)chunks.size() - 1; i >= 0; i--) { //update distances for all chunks
        NChunk* chunk = chunks[i];
        //if (chunk->freeWaiting) continue;

        chunk->distance2 = computeDistance2FromChunk(chunk->getPosition().pos, iPosition);

        // Check fade for rendering
        if ((!chunk->hasAllNeighbors() && chunk->distance2 < fadeDist2)) { // TODO(Ben): Hasmesh? 
            fadeDist2 = chunk->distance2;
        }

        // Check for container update
        if (chunk->_state > ChunkStates::TREES && !chunk->lastOwnerTask) {
            chunk->m_blocks.update(chunk->mutex);
            chunk->m_sunlight.update(chunk->mutex);
            chunk->m_lamp.update(chunk->mutex);
            chunk->m_tertiary.update(chunk->mutex);
        }

        f32 vRenderDist = (soaOptions.get(OPT_VOXEL_RENDER_DISTANCE).value.f + 36.0f);
        if (chunk->distance2 >  vRenderDist * vRenderDist) { //out of maximum range

            // Only remove it if it isn't needed by its neighbors
            if (!chunk->lastOwnerTask && !chunk->chunkDependencies) {
                if (chunk->dirty && chunk->_state > ChunkStates::TREES) {
                    m_cmp->chunkIo->addToSaveList(chunk);
                }

                freeChunk(chunk);
            }
        } else { //inside maximum range

            // Check if it is in the view frustum
            //chunk->inFrustum = frustum->sphereInFrustum(f32v3(f64v3(chunk->voxelPosition + (CHUNK_WIDTH / 2)) - position), 28.0f);

            // See if neighbors need to be added
            if (!chunk->hasAllNeighbors()) {// && chunk->needsNeighbors) {
                updateChunkNeighbors(chunk, iPosition);
            }
            //// Calculate the LOD as a power of two
            //int newLOD = (int)(sqrt(chunk->distance2) / soaOptions.get(OPT_VOXEL_LOD_THRESHOLD).value.f) + 1;
            ////  newLOD = 2;
            //if (newLOD > 6) newLOD = 6;
            //if (newLOD != chunk->getLevelOfDetail()) {
            //    chunk->setLevelOfDetail(newLOD);
            //    chunk->changeState(ChunkStates::MESH);
            //}

            if (chunk->mesh != nullptr) ChunkUpdater::randomBlockUpdates(m_cmp->physicsEngine, chunk);
            // Check to see if it needs to be added to the mesh list
            if (!chunk->_chunkListPtr && !chunk->lastOwnerTask) {
                switch (chunk->_state) {
                    case ChunkStates::WATERMESH:
                    case ChunkStates::MESH:
                        m_cmp->chunkListManager->addToMeshList(chunk);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    // Fading for voxels
    static const f32 FADE_SPEED = 0.2f;
    if (fadeDist2 == FLT_MAX) {
        ChunkRenderer::fadeDist = 0.0f;
    } else {
        f32 target = (f32)sqrt(fadeDist2) - (f32)CHUNK_WIDTH;
        ChunkRenderer::fadeDist += (target - ChunkRenderer::fadeDist) * FADE_SPEED;
    }
}

void SphericalVoxelComponentUpdater::updatePhysics(const Camera* camera) {

}

//add the loaded chunks to the setup list
void SphericalVoxelComponentUpdater::updateLoadedChunks(ui32 maxTicks) {

    ui32 startTicks = SDL_GetTicks();
    NChunk* ch;
    //IO load chunks
    while (m_cmp->chunkIo->finishedLoadChunks.try_dequeue(ch)) {

        bool canGenerate = true;
        ch->inLoadThread = 0;

        // Don't do anything if the chunk should be freed
        if (ch->freeWaiting) continue;

        //If the heightmap has not been generated, generate it.
        std::shared_ptr<ChunkGridData>& chunkGridData = ch->chunkGridData;

        //TODO(Ben): Beware of race here.
        if (!chunkGridData->isLoaded) {
            if (!chunkGridData->wasRequestSent) {
                // Keep trying to send it until it succeeds
                while (!m_cmp->generator->heightmapGenRpcDispatcher.dispatchHeightmapGen(chunkGridData,
                    ch->gridPosition, (f32)(m_cmp->planetGenData->radius * VOXELS_PER_KM)));
            }

            canGenerate = false;
        }

        // If it is not saved. Generate it!
        if (ch->loadStatus == 1) {
            // If we can generate immediately, then do so. Otherwise we wait
            if (canGenerate) {
                addGenerateTask(ch);
            } else {
                m_cmp->chunkListManager->addToGenerateList(ch);
            }
        } else {
            ch->needsNeighbors = true;
            ch->_state = ChunkStates::MESH;
            m_cmp->chunkListManager->addToMeshList(ch);
            ch->dirty = false;
            ch->isAccessible = true;
        }

        if (SDL_GetTicks() - startTicks > maxTicks) break;
    }
}

void SphericalVoxelComponentUpdater::updateGenerateList() {
    NChunk *chunk;
    std::vector<NChunk*>& generateList = m_cmp->chunkListManager->generateList;
    for (i32 i = generateList.size() - 1; i >= 0; i--) {
        chunk = generateList[i];
        // Check if the chunk is waiting to be freed
        if (chunk->freeWaiting) {
            // Remove from the setup list
            generateList[i] = generateList.back();
            generateList.pop_back();
            chunk->clearChunkListPtr();
            continue;
        } else if (chunk->chunkGridData->isLoaded) {
            addGenerateTask(chunk);
            generateList[i] = generateList.back();
            generateList.pop_back();
            chunk->clearChunkListPtr();
        }
    }
}

void SphericalVoxelComponentUpdater::updateTreesToPlace(ui32 maxTicks) {
    ui32 startTicks = SDL_GetTicks();
    NChunk* startChunk;
    std::vector<GeneratedTreeNodes*>& treesToPlace = m_cmp->chunkGrid->treesToPlace;

    for (int i = treesToPlace.size() - 1; i >= 0; i--) {
        // Check for timer end condition
        if (SDL_GetTicks() - startTicks > maxTicks) break;

        GeneratedTreeNodes* nodes = treesToPlace[i];

        if (nodes->numFrames <= 0) {
            // Check to see if initial chunk is unloaded
            startChunk = m_cmp->chunkGrid->getChunk(nodes->startChunkGridPos);
            if (startChunk == nullptr) {
                delete nodes;
                treesToPlace[i] = treesToPlace.back();
                treesToPlace.pop_back();
                continue;
            }
            // Check to see if all the chunks we need are available
            bool allChunksLoaded = true;
            for (auto& it : nodes->allChunkPositions) {
                NChunk* chunk = m_cmp->chunkGrid->getChunk(it);
                if (chunk == nullptr || chunk->isAccessible == false) {
                    allChunksLoaded = false;
                    break;
                }
            }
            // Check to see if we can now place the nodes
            if (allChunksLoaded) {
                placeTreeNodes(nodes);
                delete nodes;
                treesToPlace[i] = treesToPlace.back();
                treesToPlace.pop_back();
                // Update startChunk state 
                startChunk->_state = ChunkStates::MESH;
                m_cmp->chunkListManager->addToMeshList(startChunk);
            } else {
                // We should wait FRAMES_BEFORE_ATTEMPT frames before retrying
                nodes->numFrames = GeneratedTreeNodes::FRAMES_BEFORE_ATTEMPT;
            }
        } else {
            nodes->numFrames--;
        }
    }
}

void SphericalVoxelComponentUpdater::updateLoadList(ui32 maxTicks) {

    static std::vector<NChunk* > chunksToLoad; // Static to prevent allocation

    ui32 sticks = SDL_GetTicks();
    std::vector<NChunk*>& loadList = m_cmp->chunkListManager->loadList;
    while (!loadList.empty()) {
        NChunk* chunk = loadList.back();

        loadList.pop_back();
        chunk->clearChunkListPtr();

        // Check if the chunk is waiting to be freed
        if (chunk->freeWaiting) continue;

        chunksToLoad.push_back(chunk);

        if (SDL_GetTicks() - sticks >= maxTicks) {
            break;
        }
    }

    if (chunksToLoad.size()) m_cmp->chunkIo->addToLoadList(chunksToLoad);
    chunksToLoad.clear();
}

i32 SphericalVoxelComponentUpdater::updateSetupList(ui32 maxTicks) {
    NChunk* chunk;
    ChunkStates state;
    i32 i;
    f64v3 cpos;

    ui32 startTicks = SDL_GetTicks();
    std::vector<NChunk*>& setupList = m_cmp->chunkListManager->setupList;
    for (i = setupList.size() - 1; i >= 0; i--) {
        //limit the time
        chunk = setupList[i];
        chunk->setupWaitingTime = 0;
        state = chunk->_state;

        // Check if the chunk is waiting to be freed
        if (chunk->freeWaiting) {
            // Remove from the setup list
            setupList[i] = setupList.back();
            setupList.pop_back();
            chunk->clearChunkListPtr();
            continue;
        }

        switch (state) {
            case ChunkStates::TREES:
                if (chunk->numNeighbors == 6) {
                    FloraTask* floraTask = new FloraTask;
                    floraTask->init(chunk);
                    chunk->lastOwnerTask = floraTask;
                    m_cmp->threadPool->addTask(floraTask);
                    // Remove from the setup list
                    setupList[i] = setupList.back();
                    setupList.pop_back();
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

i32 SphericalVoxelComponentUpdater::updateMeshList(ui32 maxTicks) {

    ui32 startTicks = SDL_GetTicks();
    ChunkStates state;
    NChunk* chunk;

    RenderTask *newRenderTask;

    std::vector<NChunk*>& meshList = m_cmp->chunkListManager->meshList;
    for (i32 i = meshList.size() - 1; i >= 0; i--) {
        state = meshList[i]->_state;
        chunk = meshList[i];

        // If it is waiting to be freed, don't do anything with it
        if (chunk->freeWaiting) {
            // Remove from the mesh list
            meshList[i] = meshList.back();
            meshList.pop_back();
            chunk->clearChunkListPtr();
            continue;
        }

        // If it has no solid blocks, dont mesh it
        if (!chunk->numBlocks) {
            // Remove from the mesh list
            meshList[i] = meshList.back();
            meshList.pop_back();
            chunk->clearChunkListPtr();
            chunk->_state = ChunkStates::INACTIVE;
            continue;
        }

        if (chunk->inFrustum && trySetMeshDependencies(chunk)) {

            // Allocate mesh if needed
            if (chunk->mesh == nullptr) {
                chunk->mesh = new ChunkMesh(chunk);
            }

            // Get a render task
            newRenderTask = new RenderTask;

            if (chunk->_state == ChunkStates::MESH) {
                newRenderTask->init(chunk, RenderTaskType::DEFAULT,
                                    m_cmp->chunkMeshManager);
            } else {
                newRenderTask->init(chunk, RenderTaskType::LIQUID, m_cmp->chunkMeshManager);
            }

            chunk->lastOwnerTask = newRenderTask;
            m_cmp->threadPool->addTask(newRenderTask);

            // Remove from the mesh list
            meshList[i] = meshList.back();
            meshList.pop_back();
            chunk->clearChunkListPtr();

            chunk->_state = ChunkStates::DRAW;
        }

        if (SDL_GetTicks() - startTicks > maxTicks) break;
    }
    return 0;
}

void SphericalVoxelComponentUpdater::makeChunkAt(const ChunkPosition3D& chunkPosition) {

    // Make and initialize a chunk
    NChunk* chunk = m_cmp->chunkAllocator->getNewChunk();
    if (!chunk) return; ///< Can't make a chunk if we have no free chunks
    chunk->init(chunkPosition);

    // Add the chunkSlot to the hashmap keyed on the chunk Position
    m_cmp->chunkGrid->addChunk(chunk);

    // Mark the chunk for loading
    m_cmp->chunkListManager->addToLoadList(chunk);

    // Connect to any neighbors
    chunk->detectNeighbors(m_cmp->chunkGrid->m_chunkMap);
}

void SphericalVoxelComponentUpdater::processFinishedTasks() {

#define MAX_TASKS 100

    // Stores tasks for bulk deque
    vcore::IThreadPoolTask<WorkerData>* taskBuffer[MAX_TASKS];

    size_t numTasks = m_cmp->threadPool->getFinishedTasks(taskBuffer, MAX_TASKS);

    vcore::IThreadPoolTask<WorkerData>* task;
    NChunk* chunk;
    for (size_t i = 0; i < numTasks; i++) {
        task = taskBuffer[i];

        // Post processing based on task type
        switch (task->getTaskId()) {
            case RENDER_TASK_ID:
                chunk = static_cast<RenderTask*>(task)->chunk;
                tryRemoveMeshDependencies(chunk);
                if (task == chunk->lastOwnerTask) chunk->lastOwnerTask = nullptr;
                delete task; //TODO(Ben): Cache?
                break;
            case GENERATE_TASK_ID:
                processFinishedGenerateTask(static_cast<GenerateTask*>(task));
                delete task; //TODO(Ben): Cache?
                break;
            case FLORA_TASK_ID:
                processFinishedFloraTask(static_cast<FloraTask*>(task));
                break;
            case CA_TASK_ID:
                chunk = static_cast<CellularAutomataTask*>(task)->_chunk;
                if (task == chunk->lastOwnerTask) {
                    chunk->lastOwnerTask = nullptr;
                }
                // TODO(Ben): Cache?
                delete static_cast<CellularAutomataTask*>(task)->renderTask;
                
                m_cmp->numCaTasks--;
                delete task;
                break;
            default:
                delete task;
                break;
        }
    }
}

void SphericalVoxelComponentUpdater::processFinishedGenerateTask(GenerateTask* task) {
    NChunk *ch = task->chunk;
    if (task == ch->lastOwnerTask) ch->lastOwnerTask = nullptr;
    ch->isAccessible = true;

    if (!(ch->freeWaiting)) {

        //check to see if the top chunk has light that should end up in this chunk
        m_cmp->voxelLightEngine.checkTopForSunlight(ch);

        ch->needsNeighbors = true;
        if (ch->treesToLoad.size() || ch->plantsToLoad.size()) {
            ch->_state = ChunkStates::TREES;
            m_cmp->chunkListManager->addToSetupList(ch);
        } else {
            ch->_state = ChunkStates::MESH;
            m_cmp->chunkListManager->addToMeshList(ch);
        }
    }
}

void SphericalVoxelComponentUpdater::processFinishedFloraTask(FloraTask* task) {
    NChunk* chunk = task->chunk;
    GeneratedTreeNodes* nodes;
    if (task == chunk->lastOwnerTask) chunk->lastOwnerTask = nullptr;
    if (task->isSuccessful) {
        nodes = task->generatedTreeNodes;
        if (nodes->lnodes.size() || nodes->wnodes.size()) {
            m_cmp->chunkGrid->treesToPlace.push_back(nodes);
        } else {
            chunk->_state = ChunkStates::MESH;
            m_cmp->chunkListManager->addToMeshList(chunk);
        }
        delete task;
    } else {
        // If the task wasn't successful, add it back to the task queue so it can try again.
        task->setIsFinished(false);
        chunk->lastOwnerTask = task;
        m_cmp->threadPool->addTask(task);
    }
}

void SphericalVoxelComponentUpdater::addGenerateTask(NChunk* chunk) {
    // TODO(Ben): alternative to new?
    GenerateTask* generateTask = new GenerateTask();

    // Initialize the task
    generateTask->init(chunk, new LoadData(chunk->chunkGridData->heightData,
                       m_cmp->generator->getPlanetGenData()));
    chunk->lastOwnerTask = generateTask;
    // Add the task
    m_cmp->threadPool->addTask(generateTask);
}

void SphericalVoxelComponentUpdater::placeTreeNodes(GeneratedTreeNodes* nodes) {
    ChunkGrid* chunkGrid = m_cmp->chunkGrid;
    // Decompress all chunks to arrays for efficiency
    for (auto& it : nodes->allChunkPositions) {
        NChunk* chunk = chunkGrid->getChunk(it);
        if (chunk->_blockIDContainer.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
            chunk->_blockIDContainer.changeState(vvox::VoxelStorageState::FLAT_ARRAY, chunk->_dataLock);
        }
        if (chunk->_sunlightContainer.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
            chunk->_sunlightContainer.changeState(vvox::VoxelStorageState::FLAT_ARRAY, chunk->_dataLock);
        }
    }

    int blockIndex;
    NChunk* owner;
    NChunk* lockedChunk = nullptr;
    const i32v3& startPos = nodes->startChunkGridPos;

    int a = 0;
    for (auto& node : nodes->wnodes) { //wood nodes
        blockIndex = node.blockIndex;

        owner = chunkGrid->getChunk(startPos + FloraTask::getChunkOffset(node.chunkOffset));
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
        owner = chunkGrid->getChunk(startPos + FloraTask::getChunkOffset(node.chunkOffset));
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

void SphericalVoxelComponentUpdater::freeChunk(NChunk* chunk) {
    if (chunk) {
        if (chunk->dirty && chunk->_state > ChunkStates::TREES) {
            m_cmp->chunkIo->addToSaveList(chunk);
        }

        if (chunk->inSaveThread || chunk->inLoadThread || chunk->_chunkListPtr || chunk->lastOwnerTask ||
            (chunk->mesh && chunk->mesh->refCount)) {
            // Mark the chunk as waiting to be finished with threads and add to threadWaiting list
            chunk->distance2 = 0; // make its distance 0 so it gets processed first in the lists and gets removed

            m_cmp->chunkListManager->addToFreeWaitList(chunk);
        } else {


            m_cmp->chunkGrid->removeChunk(chunk);

            chunk->clearNeighbors();
           
            // Completely clear the chunk and then recycle it 
            chunk->clear();
            m_cmp->chunkMemoryManager->freeChunk(chunk);
        }
    }
}

void SphericalVoxelComponentUpdater::updateChunkNeighbors(NChunk* chunk, const i32v3& cameraPos) {
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

void SphericalVoxelComponentUpdater::tryLoadChunkNeighbor(NChunk* chunk, const i32v3& cameraPos, const i32v3& offset) {
    ChunkPosition3D newPosition = chunk->getPosition();
    newPosition.pos += offset;

    // TODO(Ben): Redundant with elsewhere
    f32 vRenderDist = soaOptions.get(OPT_VOXEL_RENDER_DISTANCE).value.f + (f32)CHUNK_WIDTH;
    double dist2 = computeDistance2FromChunk(newPosition.pos * CHUNK_WIDTH, cameraPos);
    if (dist2 <= (vRenderDist) * (vRenderDist)) {
        makeChunkAt(newPosition);
    }
}

bool SphericalVoxelComponentUpdater::trySetMeshDependencies(NChunk* chunk) {
    // If this chunk is still in a mesh thread, don't re-add dependencies
    if (chunk->meshJobCounter) {
        chunk->meshJobCounter++;
        return true;
    }
    if (!chunk->hasAllNeighbors()) return false;

    NChunk* nc;

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

void SphericalVoxelComponentUpdater::tryRemoveMeshDependencies(NChunk* chunk) {
    chunk->meshJobCounter--;
    // If this chunk is still in a mesh thread, don't remove dependencies
    if (chunk->meshJobCounter) return;

    NChunk* nc;
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