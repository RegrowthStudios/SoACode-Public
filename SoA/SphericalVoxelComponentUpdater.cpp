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
    if (spaceSystem->sphericalVoxel.getComponentListSize() > 1) {

        // TODO(Ben): This is temporary hard coded player stuff.
        auto& playerPosCmp = gameSystem->voxelPosition.getFromEntity(soaState->playerEntity);

        for (auto& it : spaceSystem->sphericalVoxel) {
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

    // TODO(Ben): This doesn't scale for multiple agents
    m_cmp->chunkGrids[agentPosition.face].update();
}

void SphericalVoxelComponentUpdater::updateChunks(ChunkGrid& grid, const VoxelPosition3D& agentPosition) {
    // Get render distance squared
    f32 renderDist2 = (soaOptions.get(OPT_VOXEL_RENDER_DISTANCE).value.f + (f32)CHUNK_WIDTH);
    renderDist2 *= renderDist2;

    // Loop through all currently active chunks
    // TODO(Ben): Chunk should only become active after load?
    const std::vector<ChunkHandle>& chunks = grid.getActiveChunks();
    for (int i = (int)chunks.size() - 1; i >= 0; i) {
        ChunkHandle chunk = chunks[i];
        // Calculate distance TODO(Ben): Maybe don't calculate this every frame? Or use sphere approx?
        chunk->distance2 = computeDistance2FromChunk(chunk->getVoxelPosition().pos, agentPosition.pos);

        // Check container update
        if (chunk->genLevel == GEN_DONE) {
        //    chunk->updateContainers();
        }

        // Check for unload
        if (chunk->distance2 > renderDist2) {
            if (chunk->m_inLoadRange) {
                // Unload the chunk
                disposeChunkMesh(chunk);
                chunk->m_inLoadRange = false;
                grid.removeChunk(chunk, i);
            }
        } else {
            // Check for neighbor loading TODO(Ben): Don't keep redundantly checking edges? Distance threshold?
            if (chunk->m_inLoadRange) {
                if (!chunk->hasAllNeighbors()) {
                    if (chunk->genLevel > GEN_TERRAIN) {
                        tryLoadChunkNeighbors(chunk, agentPosition, renderDist2);
                    }
                } else if (chunk->genLevel == GEN_DONE && chunk->needsRemesh()) {
                    requestChunkMesh(chunk);
                }
            } else {
                chunk->m_inLoadRange = true;
                grid.addChunk(chunk);
            }
        }
    }
}

void SphericalVoxelComponentUpdater::tryLoadChunkNeighbors(Chunk* chunk, const VoxelPosition3D& agentPosition, f32 loadDist2) {
    const f64v3& pos = chunk->getVoxelPosition().pos;
    if (!chunk->left.isAquired()) {
        f64v3 newPos(pos.x - (f64)CHUNK_WIDTH, pos.y, pos.z);
        tryLoadChunkNeighbor(agentPosition, loadDist2, newPos);
    }
    if (!chunk->right.isAquired()) {
        f64v3 newPos(pos.x + (f64)CHUNK_WIDTH, pos.y, pos.z);
        tryLoadChunkNeighbor(agentPosition, loadDist2, newPos);
    }
    if (!chunk->bottom.isAquired()) {
        f64v3 newPos(pos.x, pos.y - (f64)CHUNK_WIDTH, pos.z);
        tryLoadChunkNeighbor(agentPosition, loadDist2, newPos);
    }
    if (!chunk->top.isAquired()) {
        f64v3 newPos(pos.x, pos.y + (f64)CHUNK_WIDTH, pos.z);
        tryLoadChunkNeighbor(agentPosition, loadDist2, newPos);
    }
    if (!chunk->back.isAquired()) {
        f64v3 newPos(pos.x, pos.y, pos.z - (f64)CHUNK_WIDTH);
        tryLoadChunkNeighbor(agentPosition, loadDist2, newPos);
    }
    if (!chunk->front.isAquired()) {
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
        q->chunk->m_inLoadRange = true;
    }
}

void SphericalVoxelComponentUpdater::requestChunkMesh(ChunkHandle chunk) {
    // If it has no solid blocks, don't mesh it
    if (chunk->numBlocks <= 0) {
        chunk->remeshFlags = 0;
        return;
    }

    if (/*chunk->inFrustum && */!chunk->queuedForMesh && trySetMeshDependencies(chunk)) {

        ChunkMeshTask* newRenderTask = new ChunkMeshTask;

        newRenderTask->init(chunk, MeshTaskType::DEFAULT, m_cmp->blockPack, m_cmp->chunkMeshManager);

        m_cmp->threadPool->addTask(newRenderTask);

        chunk->remeshFlags = 0;
    }
}

void SphericalVoxelComponentUpdater::disposeChunkMesh(Chunk* chunk) {
    // delete the mesh!
    if (chunk->hasCreatedMesh) {
        ChunkMeshMessage msg;
        msg.chunkID = chunk->getID();
        msg.messageID = ChunkMeshMessageID::DESTROY;
        m_cmp->chunkMeshManager->sendMessage(msg);
        chunk->hasCreatedMesh = true;
    }
}

bool SphericalVoxelComponentUpdater::trySetMeshDependencies(ChunkHandle chunk) {

    ChunkHandle left = chunk->left;
    ChunkHandle right = chunk->right;
    ChunkHandle bottom = chunk->bottom;
    ChunkHandle top = chunk->top;
    ChunkHandle back = chunk->back;
    ChunkHandle front = chunk->front;

    //// Check that neighbors are loaded
    if (!left->isAccessible || !right->isAccessible ||
        !front->isAccessible || !back->isAccessible ||
        !top->isAccessible || !bottom->isAccessible) return false;
    
    // Left half
    if (!left->back.isAquired() || !left->back->isAccessible) return false;
    if (!left->front.isAquired() || !left->front->isAccessible) return false;

    ChunkHandle leftTop = left->top;
    ChunkHandle leftBot = left->bottom;
    if (!leftTop.isAquired() || !leftTop->isAccessible) return false;
    if (!leftBot.isAquired() || !leftBot->isAccessible) return false;
    if (!leftTop->back.isAquired() || !leftTop->back->isAccessible) return false;
    if (!leftTop->front.isAquired() || !leftTop->front->isAccessible) return false;
    if (!leftBot->back.isAquired() || !leftBot->back->isAccessible) return false;
    if (!leftBot->front.isAquired() || !leftBot->front->isAccessible) return false;

    // right half
    if (!right->back.isAquired() || !right->back->isAccessible) return false;
    if (!right->front.isAquired() || !right->front->isAccessible) return false;

    ChunkHandle rightTop = right->top;
    ChunkHandle rightBot = right->bottom;
    if (!rightTop.isAquired() || !rightTop->isAccessible) return false;
    if (!rightBot.isAquired() || !rightBot->isAccessible) return false;
    if (!rightTop->back.isAquired() || !rightTop->back->isAccessible) return false;
    if (!rightTop->front.isAquired() || !rightTop->front->isAccessible) return false;
    if (!rightBot->back.isAquired() || !rightBot->back->isAccessible) return false;
    if (!rightBot->front.isAquired() || !rightBot->front->isAccessible) return false;

    if (!top->back.isAquired() || !top->back->isAccessible) return false;
    if (!top->front.isAquired() || !top->front->isAccessible) return false;

    if (!bottom->back.isAquired() || !bottom->back->isAccessible) return false;
    if (!bottom->front.isAquired() || !bottom->front->isAccessible) return false;

    // Set dependencies
    chunk.acquire();
    left.acquire();
    right.acquire();
    chunk->front.acquire();
    chunk->back.acquire();
    chunk->top.acquire();
    chunk->bottom.acquire();
    left->back.acquire();
    left->front.acquire();
    leftTop.acquire();
    leftBot.acquire();
    leftTop->back.acquire();
    leftTop->front.acquire();
    leftBot->back.acquire();
    leftBot->front.acquire();
    right->back.acquire();
    right->front.acquire();
    rightTop.acquire();
    rightBot.acquire();
    rightTop->back.acquire();
    rightTop->front.acquire();
    rightBot->back.acquire();
    rightBot->front.acquire();
    top->back.acquire();
    top->front.acquire();
    bottom->back.acquire();
    bottom->front.acquire();

    return true;
}

