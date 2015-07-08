#include "stdafx.h"
#include "SphericalVoxelComponentUpdater.h"

#include <SDL/SDL_timer.h> // For SDL_GetTicks

#include "Chunk.h"
#include "ChunkAllocator.h"
#include "ChunkGrid.h"
#include "ChunkIOManager.h"
#include "ChunkMeshManager.h"
#include "ChunkMeshTask.h"
#include "ChunkRenderer.h"
#include "ChunkUpdater.h"
#include "FloraTask.h"
#include "GameSystem.h"
#include "GenerateTask.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "PlanetGenData.h"
#include "SoaOptions.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "SpaceSystemComponents.h"
#include "VoxelSpaceConversions.h"
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
    // Always make a chunk at camera location
    i32v3 chunkPosition = VoxelSpaceConversions::voxelToChunk(agentPosition.pos);
    if (m_cmp->chunkGrids[agentPosition.face].getChunk(chunkPosition) == nullptr) {
        // TODO(Ben): Minimize new calls
        ChunkQuery* q = new ChunkQuery;
        q->set(chunkPosition, GEN_DONE, true);
        m_cmp->chunkGrids[agentPosition.face].submitQuery(q);
    }

    updateChunks(m_cmp->chunkGrids[agentPosition.face], agentPosition);

    // Clear mesh dependencies
#define MAX_MESH_DEPS_CHUNKS 200
    Chunk* buffer[MAX_MESH_DEPS_CHUNKS];
    if (size_t numFlushed = m_cmp->meshDepsFlushList->try_dequeue_bulk(buffer, MAX_MESH_DEPS_CHUNKS)) {
        for (size_t i = 0; i < numFlushed; i++) {
            removeMeshDependencies(buffer[i]);
        }
    }

    // TODO(Ben): This doesn't scale for multiple agents
    m_cmp->chunkGrids[agentPosition.face].update();
}

void SphericalVoxelComponentUpdater::updateChunks(ChunkGrid& grid, const VoxelPosition3D& agentPosition) {
    // Get render distance squared
    f32 renderDist2 = (soaOptions.get(OPT_VOXEL_RENDER_DISTANCE).value.f + (f32)CHUNK_WIDTH);
    renderDist2 *= renderDist2;

    // Loop through all currently active chunks
    // TODO(Ben): Chunk should only become active after load?
    const std::vector<Chunk*>& chunks = grid.getActiveChunks();
    for (int i = (int)chunks.size() - 1; i >= 0; i--) {
        Chunk* chunk = chunks[i];
        // Calculate distance TODO(Ben): Maybe don't calculate this every frame? Or use sphere approx?
        chunk->distance2 = computeDistance2FromChunk(chunk->getVoxelPosition().pos, agentPosition.pos);

        // Check container update
        if (chunk->genLevel == GEN_DONE) {
        //    chunk->updateContainers();
        }

        // Check for unload
        if (chunk->distance2 > renderDist2) {
            if (chunk->refCount == 0) {
                // Unload the chunk
                disposeChunk(chunk);
                grid.removeChunk(chunk, i);
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
    if (chunk->numBlocks <= 0) {
        chunk->remeshFlags = 0;
        return;
    }

    if (/*chunk->inFrustum && */!chunk->queuedForMesh && trySetMeshDependencies(chunk)) {

        ChunkMeshTask* newRenderTask = new ChunkMeshTask;

        newRenderTask->init(chunk, MeshTaskType::DEFAULT, m_cmp->meshDepsFlushList, m_cmp->blockPack, m_cmp->chunkMeshManager);

        chunk->refCount++;
        m_cmp->threadPool->addTask(newRenderTask);

        chunk->remeshFlags = 0;
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

    Chunk* left = chunk->left;
    Chunk* right = chunk->right;
    Chunk* bottom = chunk->bottom;
    Chunk* top = chunk->top;
    Chunk* back = chunk->back;
    Chunk* front = chunk->front;

    //// Check that neighbors are loaded
    if (!left->isAccessible || !right->isAccessible ||
        !front->isAccessible || !back->isAccessible ||
        !top->isAccessible || !bottom->isAccessible) return false;
    
    // Left half
    if (!left->back || !left->back->isAccessible) return false;
    if (!left->front || !left->front->isAccessible) return false;

    Chunk* leftTop = left->top;
    Chunk* leftBot = left->bottom;
    if (!leftTop || !leftTop->isAccessible) return false;
    if (!leftBot || !leftBot->isAccessible) return false;
    if (!leftTop->back || !leftTop->back->isAccessible) return false;
    if (!leftTop->front || !leftTop->front->isAccessible) return false;
    if (!leftBot->back || !leftBot->back->isAccessible) return false;
    if (!leftBot->front || !leftBot->front->isAccessible) return false;

    // right half
    if (!right->back || !right->back->isAccessible) return false;
    if (!right->front || !right->front->isAccessible) return false;

    Chunk* rightTop = right->top;
    Chunk* rightBot = right->bottom;
    if (!rightTop || !rightTop->isAccessible) return false;
    if (!rightBot || !rightBot->isAccessible) return false;
    if (!rightTop->back || !rightTop->back->isAccessible) return false;
    if (!rightTop->front || !rightTop->front->isAccessible) return false;
    if (!rightBot->back || !rightBot->back->isAccessible) return false;
    if (!rightBot->front || !rightBot->front->isAccessible) return false;

    if (!top->back || !top->back->isAccessible) return false;
    if (!top->front || !top->front->isAccessible) return false;

    if (!bottom->back || !bottom->back->isAccessible) return false;
    if (!bottom->front || !bottom->front->isAccessible) return false;

    // Set dependencies
    ++chunk->left->refCount;
    ++chunk->right->refCount;
    ++chunk->front->refCount;
    ++chunk->back->refCount;
    ++chunk->top->refCount;
    ++chunk->bottom->refCount;

    ++left->back->refCount;
    ++left->front->refCount;

    ++leftTop->refCount;
    ++leftBot->refCount;
    ++leftTop->back->refCount;
    ++leftTop->front->refCount;
    ++leftBot->back->refCount;
    ++leftBot->front->refCount;

    ++right->back->refCount;
    ++right->front->refCount;

    ++rightTop->refCount;
    ++rightBot->refCount;
    ++rightTop->back->refCount;
    ++rightTop->front->refCount;
    ++rightBot->back->refCount;
    ++rightBot->front->refCount;

    ++top->back->refCount;
    ++top->front->refCount;

    ++bottom->back->refCount;
    ++bottom->front->refCount;

    return true;
}

void SphericalVoxelComponentUpdater::removeMeshDependencies(Chunk* chunk) {

    Chunk* left = chunk->left;
    Chunk* right = chunk->right;
    Chunk* bottom = chunk->bottom;
    Chunk* top = chunk->top;
    Chunk* back = chunk->back;
    Chunk* front = chunk->front;
    Chunk* leftTop = left->top;
    Chunk* leftBot = left->bottom;
    Chunk* rightTop = right->top;
    Chunk* rightBot = right->bottom;

    --chunk->refCount;

    // Remove dependencies
    --chunk->left->refCount;
    --chunk->right->refCount;
    --chunk->front->refCount;
    --chunk->back->refCount;
    --chunk->top->refCount;
    --chunk->bottom->refCount;

    --left->back->refCount;
    --left->front->refCount;

    --leftTop->refCount;
    --leftBot->refCount;
    --leftTop->back->refCount;
    --leftTop->front->refCount;
    --leftBot->back->refCount;
    --leftBot->front->refCount;

    --right->back->refCount;
    --right->front->refCount;

    --rightTop->refCount;
    --rightBot->refCount;
    --rightTop->back->refCount;
    --rightTop->front->refCount;
    --rightBot->back->refCount;
    --rightBot->front->refCount;

    --top->back->refCount;
    --top->front->refCount;

    --bottom->back->refCount;
    --bottom->front->refCount;
}