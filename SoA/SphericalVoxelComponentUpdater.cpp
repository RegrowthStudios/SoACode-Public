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
    if (chunkPosition != m_lastChunkPos) {
        if (m_centerHandle.isAquired()) m_centerHandle.release();
        // TODO(Ben): Minimize new calls
        ChunkQuery* q = new ChunkQuery;
        q->set(chunkPosition, GEN_DONE, true);
        m_cmp->chunkGrids[agentPosition.face].submitQuery(q);
        m_centerHandle = q->chunk.acquire();
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
    for (int i = (int)chunks.size() - 1; i >= 0; i--) {
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

    if (/*chunk->inFrustum && */!chunk->queuedForMesh) {
        ChunkMeshTask* meshTask = trySetMeshDependencies(chunk);

        if (meshTask) {
            m_cmp->threadPool->addTask(meshTask);
            chunk->remeshFlags = 0;
        }
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

ChunkMeshTask* SphericalVoxelComponentUpdater::trySetMeshDependencies(ChunkHandle chunk) {

    ChunkHandle left = chunk->left;
    ChunkHandle right = chunk->right;
    ChunkHandle bottom = chunk->bottom;
    ChunkHandle top = chunk->top;
    ChunkHandle back = chunk->back;
    ChunkHandle front = chunk->front;

    //// Check that neighbors are loaded
    if (!left->isAccessible || !right->isAccessible ||
        !front->isAccessible || !back->isAccessible ||
        !top->isAccessible || !bottom->isAccessible) return nullptr;
    
    // Left half
    if (!left->back.isAquired() || !left->back->isAccessible) return nullptr;
    if (!left->front.isAquired() || !left->front->isAccessible) return nullptr;

    ChunkHandle leftTop = left->top;
    ChunkHandle leftBot = left->bottom;
    if (!leftTop.isAquired() || !leftTop->isAccessible) return nullptr;
    if (!leftBot.isAquired() || !leftBot->isAccessible) return nullptr;
    if (!leftTop->back.isAquired() || !leftTop->back->isAccessible) return nullptr;
    if (!leftTop->front.isAquired() || !leftTop->front->isAccessible) return nullptr;
    if (!leftBot->back.isAquired() || !leftBot->back->isAccessible) return nullptr;
    if (!leftBot->front.isAquired() || !leftBot->front->isAccessible) return nullptr;

    // right half
    if (!right->back.isAquired() || !right->back->isAccessible) return nullptr;
    if (!right->front.isAquired() || !right->front->isAccessible) return nullptr;

    ChunkHandle rightTop = right->top;
    ChunkHandle rightBot = right->bottom;
    if (!rightTop.isAquired() || !rightTop->isAccessible) return nullptr;
    if (!rightBot.isAquired() || !rightBot->isAccessible) return nullptr;
    if (!rightTop->back.isAquired() || !rightTop->back->isAccessible) return nullptr;
    if (!rightTop->front.isAquired() || !rightTop->front->isAccessible) return nullptr;
    if (!rightBot->back.isAquired() || !rightBot->back->isAccessible) return nullptr;
    if (!rightBot->front.isAquired() || !rightBot->front->isAccessible) return nullptr;

    if (!top->back.isAquired() || !top->back->isAccessible) return nullptr;
    if (!top->front.isAquired() || !top->front->isAccessible) return nullptr;

    if (!bottom->back.isAquired() || !bottom->back->isAccessible) return nullptr;
    if (!bottom->front.isAquired() || !bottom->front->isAccessible) return nullptr;

    // TODO(Ben): Recycler
    ChunkMeshTask* meshTask = new ChunkMeshTask;
    meshTask->init(chunk, MeshTaskType::DEFAULT, m_cmp->blockPack, m_cmp->chunkMeshManager);

    // Set dependencies
    meshTask->chunk.acquire();
    meshTask->neighborHandles[0] = left.acquire();
    meshTask->neighborHandles[1] = right.acquire();
    meshTask->neighborHandles[2] = chunk->front.acquire();
    meshTask->neighborHandles[3] = chunk->back.acquire();
    meshTask->neighborHandles[4] = chunk->top.acquire();
    meshTask->neighborHandles[5] = chunk->bottom.acquire();
    meshTask->neighborHandles[6] = left->back.acquire();
    meshTask->neighborHandles[7] = left->front.acquire();
    meshTask->neighborHandles[8] = leftTop.acquire();
    meshTask->neighborHandles[9] = leftBot.acquire();
    meshTask->neighborHandles[10] = leftTop->back.acquire();
    meshTask->neighborHandles[11] = leftTop->front.acquire();
    meshTask->neighborHandles[12] = leftBot->back.acquire();
    meshTask->neighborHandles[13] = leftBot->front.acquire();
    meshTask->neighborHandles[14] = right->back.acquire();
    meshTask->neighborHandles[15] = right->front.acquire();
    meshTask->neighborHandles[16] = rightTop.acquire();
    meshTask->neighborHandles[17] = rightBot.acquire();
    meshTask->neighborHandles[18] = rightTop->back.acquire();
    meshTask->neighborHandles[19] = rightTop->front.acquire();
    meshTask->neighborHandles[20] = rightBot->back.acquire();
    meshTask->neighborHandles[21] = rightBot->front.acquire();
    meshTask->neighborHandles[22] = top->back.acquire();
    meshTask->neighborHandles[23] = top->front.acquire();
    meshTask->neighborHandles[24] = bottom->back.acquire();
    meshTask->neighborHandles[25] = bottom->front.acquire();

    return meshTask;
}

