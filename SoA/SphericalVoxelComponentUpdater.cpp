#include "stdafx.h"
#include "SphericalVoxelComponentUpdater.h"

#include <SDL/SDL_timer.h> // For SDL_GetTicks

#include "ChunkAllocator.h"
#include "ChunkIOManager.h"
#include "ChunkMeshManager.h"
#include "ChunkRenderer.h"
#include "ChunkUpdater.h"
#include "FloraTask.h"
#include "GameSystem.h"
#include "GenerateTask.h"
#include "Chunk.h"
#include "ChunkGrid.h"
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
    SpaceSystem* spaceSystem = soaState->spaceSystem;
    GameSystem* gameSystem = soaState->gameSystem;
    if (spaceSystem->m_sphericalVoxelCT.getComponentListSize() > 1) {

        // TODO(Ben): This is temporary hard coded player stuff.
        auto& playerPosCmp = gameSystem->voxelPosition.getFromEntity(soaState->playerEntity);
        auto& playerFrustumCmp = gameSystem->frustum.getFromEntity(soaState->playerEntity);

        for (auto& it : spaceSystem->m_sphericalVoxelCT) {
            if (it.second.chunkGrids) {
                m_cmp = &it.second;
                updateComponent(playerPosCmp.gridPosition);
            }
        }
    }
}

void SphericalVoxelComponentUpdater::updateComponent(const VoxelPosition3D& agentPosition) {
    return; // TODO(Ben): Temporary
    
    // Always make a chunk at camera location
    i32v3 chunkPosition = VoxelSpaceConversions::voxelToChunk(agentPosition.pos);
    if (m_cmp->chunkGrids[agentPosition.face].getChunk(chunkPosition) == nullptr) {
        // TODO(Ben): Minimize new calls
        ChunkQuery* q = new ChunkQuery;
        q->set(chunkPosition, GEN_DONE, true);
        m_cmp->chunkGrids[agentPosition.face].submitQuery(q);
    }

    updateChunks(m_cmp->chunkGrids[agentPosition.face], agentPosition);

    // TODO(Ben): This doesn't scale for multiple agents
    m_cmp->chunkGrids[agentPosition.face].update();
}

void SphericalVoxelComponentUpdater::updateChunks(ChunkGrid& grid, const VoxelPosition3D& agentPosition) {
    // Get render distance squared
    f32 renderDist2 = (soaOptions.get(OPT_VOXEL_RENDER_DISTANCE).value.f + (f32)CHUNK_WIDTH);
    renderDist2 *= renderDist2;

    // Loop through all currently active chunks
    // TODO(Ben): Chunk should only become active after load?
    Chunk* chunk = grid.getActiveChunks();
    while (chunk != nullptr) {

        // Calculate distance TODO(Ben): Maybe don't calculate this every frame? Or use sphere approx?
        chunk->m_distance2 = computeDistance2FromChunk(chunk->getVoxelPosition().pos, agentPosition.pos);

        // Check container update
        if (chunk->genLevel == GEN_DONE) {
        //    chunk->updateContainers();
        }

        // Check for unload
        if (chunk->m_distance2 > renderDist2) {
            if (chunk->refCount == 0) {
                // Unload the chunk
                Chunk* tmp = chunk;
                chunk = chunk->getNextActive();
                disposeChunk(tmp);
                grid.removeChunk(tmp);
            } else {
                chunk = chunk->getNextActive();
            }
        } else {
            // Check for neighbor loading TODO(Ben): Don't keep redundantly checking edges? Distance threshold?
            if (!chunk->hasAllNeighbors()){
                if (chunk->genLevel > GEN_TERRAIN) {
                    tryLoadChunkNeighbors(chunk, agentPosition, renderDist2);
                }
            } else if (chunk->genLevel == GEN_DONE && chunk->needsRemesh()) {
                requestChunkMesh(chunk);
            }
            chunk = chunk->getNextActive();
        }
    }
}

void SphericalVoxelComponentUpdater::tryLoadChunkNeighbors(Chunk* chunk, const VoxelPosition3D& agentPosition, f32 loadDist2) {
    const f64v3& pos = chunk->getVoxelPosition().pos;
    if (!chunk->left) {
        f64v3 newPos(pos.x - (f64)CHUNK_WIDTH, pos.y, pos.z);
        tryLoadChunkNeighbor(agentPosition, loadDist2, newPos);
    }
    if (!chunk->right) {
        f64v3 newPos(pos.x + (f64)CHUNK_WIDTH, pos.y, pos.z);
        tryLoadChunkNeighbor(agentPosition, loadDist2, newPos);
    }
    if (!chunk->bottom) {
        f64v3 newPos(pos.x, pos.y - (f64)CHUNK_WIDTH, pos.z);
        tryLoadChunkNeighbor(agentPosition, loadDist2, newPos);
    }
    if (!chunk->top) {
        f64v3 newPos(pos.x, pos.y + (f64)CHUNK_WIDTH, pos.z);
        tryLoadChunkNeighbor(agentPosition, loadDist2, newPos);
    }
    if (!chunk->back) {
        f64v3 newPos(pos.x, pos.y, pos.z - (f64)CHUNK_WIDTH);
        tryLoadChunkNeighbor(agentPosition, loadDist2, newPos);
    }
    if (!chunk->front) {
        f64v3 newPos(pos.x, pos.y, pos.z + (f64)CHUNK_WIDTH);
        tryLoadChunkNeighbor(agentPosition, loadDist2, newPos);
    }
    
}

void SphericalVoxelComponentUpdater::tryLoadChunkNeighbor(const VoxelPosition3D& agentPosition, f32 loadDist2, const f64v3& pos) {
    f32 dist2 = computeDistance2FromChunk(pos, agentPosition.pos);
    if (dist2 <= loadDist2) {
        ChunkQuery* q = new ChunkQuery;
        q->set(VoxelSpaceConversions::voxelToChunk(pos), GEN_DONE, true);
        m_cmp->chunkGrids[agentPosition.face].submitQuery(q);
    }
}

void SphericalVoxelComponentUpdater::requestChunkMesh(Chunk* chunk) {
    // If it has no solid blocks, don't mesh it
    //if (!chunk->numBlocks) {
    //    // Remove from the mesh list
    //    meshList[i] = meshList.back();
    //    meshList.pop_back();
    //    chunk->clearChunkListPtr();
    //    chunk->_state = ChunkStates::INACTIVE;
    //    continue;
    //}

    if (/*chunk->inFrustum && */!chunk->queuedForMesh && trySetMeshDependencies(chunk)) {

       

        // Get a render task
        // TODO(Ben): This is a purposeful, temporary memory leak. Don't freak out
        RenderTask* newRenderTask = new RenderTask;
        
        newRenderTask->init(chunk, RenderTaskType::DEFAULT, m_cmp->blockPack, m_cmp->chunkMeshManager);

        chunk->refCount++;
        m_cmp->threadPool->addTask(newRenderTask);

        chunk->m_remeshFlags = 0;
    }
}

void SphericalVoxelComponentUpdater::disposeChunk(Chunk* chunk) {
    // delete the mesh!
    if (chunk->hasCreatedMesh) {
        ChunkMeshMessage msg;
        msg.chunkID = chunk->getID();
        msg.messageID = ChunkMeshMessageID::DESTROY;
        m_cmp->chunkMeshManager->sendMessage(msg);
        chunk->hasCreatedMesh = true;
    }
}

bool SphericalVoxelComponentUpdater::trySetMeshDependencies(Chunk* chunk) {
    //// TODO(Ben): This is a lot of branching. There must be a better way
    //// If this chunk is still in a mesh thread, don't re-add dependencies
    //if (chunk->meshJobCounter) {
    //    chunk->meshJobCounter++;
    //    return true;
    //}

    //NChunk* nc;

    //// Neighbors
    //if (!chunk->left->isAccessible || !chunk->right->isAccessible ||
    //    !chunk->front->isAccessible || !chunk->back->isAccessible ||
    //    !chunk->top->isAccessible || !chunk->bottom->isAccessible) return false;

    //// Left Side
    //if (!chunk->left->back || !chunk->left->back->isAccessible) return false;
    //if (!chunk->left->front || !chunk->left->front->isAccessible) return false;
    //nc = chunk->left->top;
    //if (!nc || !nc->isAccessible) return false;
    //if (!nc->back || !nc->back->isAccessible) return false;
    //if (!nc->front || !nc->front->isAccessible) return false;
    //nc = chunk->left->bottom;
    //if (!nc || !nc->isAccessible) return false;
    //if (!nc->back || !nc->back->isAccessible) return false;
    //if (!nc->front || !nc->front->isAccessible) return false;

    //// Right side
    //if (!chunk->right->back || !chunk->right->back->isAccessible) return false;
    //if (!chunk->right->front || !chunk->right->front->isAccessible) return false;
    //nc = chunk->right->top;
    //if (!nc || !nc->isAccessible) return false;
    //if (!nc->back || !nc->back->isAccessible) return false;
    //if (!nc->front || !nc->front->isAccessible) return false;
    //nc = chunk->right->bottom;
    //if (!nc || !nc->isAccessible) return false;
    //if (!nc->back || !nc->back->isAccessible) return false;
    //if (!nc->front || !nc->front->isAccessible) return false;

    //// Front
    //if (!chunk->front->top || !chunk->front->top->isAccessible) return false;
    //if (!chunk->front->bottom || !chunk->front->bottom->isAccessible) return false;

    //// Back
    //if (!chunk->back->top || !chunk->back->top->isAccessible) return false;
    //if (!chunk->back->bottom || !chunk->back->bottom->isAccessible) return false;

    //// If we get here, we can set dependencies

    //// Neighbors
    //chunk->left->addDependency();
    //chunk->right->addDependency();
    //chunk->front->addDependency();
    //chunk->back->addDependency();
    //chunk->top->addDependency();
    //chunk->bottom->addDependency();

    //// Left Side
    //chunk->left->back->addDependency();
    //chunk->left->front->addDependency();
    //nc = chunk->left->top;
    //nc->addDependency();
    //nc->back->addDependency();
    //nc->front->addDependency();
    //nc = chunk->left->bottom;
    //nc->addDependency();
    //nc->back->addDependency();
    //nc->front->addDependency();

    //// Right side
    //chunk->right->back->addDependency();
    //chunk->right->front->addDependency();
    //nc = chunk->right->top;
    //nc->addDependency();
    //nc->back->addDependency();
    //nc->front->addDependency();
    //nc = chunk->right->bottom;
    //nc->addDependency();
    //nc->back->addDependency();
    //nc->front->addDependency();

    //// Front
    //chunk->front->top->addDependency();
    //chunk->front->bottom->addDependency();

    //// Back
    //chunk->back->top->addDependency();
    //chunk->back->bottom->addDependency();

    //chunk->meshJobCounter++;
    return true;
}

void SphericalVoxelComponentUpdater::tryRemoveMeshDependencies(Chunk* chunk) {
    //chunk->meshJobCounter--;
    //// If this chunk is still in a mesh thread, don't remove dependencies
    //if (chunk->meshJobCounter) return;

    //Chunk* nc;
    //// Neighbors
    //chunk->left->removeDependency();
    //chunk->right->removeDependency();
    //chunk->front->removeDependency();
    //chunk->back->removeDependency();
    //chunk->top->removeDependency();
    //chunk->bottom->removeDependency();

    //// Left Side
    //chunk->left->back->removeDependency();
    //chunk->left->front->removeDependency();
    //nc = chunk->left->top;
    //nc->removeDependency();
    //nc->back->removeDependency();
    //nc->front->removeDependency();
    //nc = chunk->left->bottom;
    //nc->removeDependency();
    //nc->back->removeDependency();
    //nc->front->removeDependency();

    //// Right side
    //chunk->right->back->removeDependency();
    //chunk->right->front->removeDependency();
    //nc = chunk->right->top;
    //nc->removeDependency();
    //nc->back->removeDependency();
    //nc->front->removeDependency();
    //nc = chunk->right->bottom;
    //nc->removeDependency();
    //nc->back->removeDependency();
    //nc->front->removeDependency();

    //// Front
    //chunk->front->top->removeDependency();
    //chunk->front->bottom->removeDependency();

    //// Back
    //chunk->back->top->removeDependency();
    //chunk->back->bottom->removeDependency();
}