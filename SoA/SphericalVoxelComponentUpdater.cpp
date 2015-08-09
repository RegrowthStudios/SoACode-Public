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
    
    i32v3 chunkPosition = VoxelSpaceConversions::voxelToChunk(agentPosition);
    // Don't generate when moving too fast
    bool doGen = true;
    if (glm::length(m_lastAgentPos - agentPosition.pos) > 8.0f) {
        doGen = false;
    }
    m_lastAgentPos = agentPosition;

    if (chunkPosition != m_lastChunkPos) {
        m_lastChunkPos = chunkPosition;
        // Submit a generation query
        m_cmp->chunkGrids[agentPosition.face].submitQuery(chunkPosition, GEN_DONE, true);
    }

    updateChunks(m_cmp->chunkGrids[agentPosition.face], agentPosition, doGen);

    // TODO(Ben): This doesn't scale for multiple agents
    m_cmp->chunkGrids[agentPosition.face].update();
}

void SphericalVoxelComponentUpdater::updateChunks(ChunkGrid& grid, const VoxelPosition3D& agentPosition, bool doGen) {
    // Get render distance squared
    f32 renderDist2 = (soaOptions.get(OPT_VOXEL_RENDER_DISTANCE).value.f + (f32)CHUNK_WIDTH);
    renderDist2 *= renderDist2;

    // Loop through all currently active chunks
    // TODO(Ben): Chunk should only become active after load?
    std::vector<ChunkHandle> disconnects;
    std::vector<ChunkHandle> connects;
    const std::vector<ChunkHandle>& chunks = grid.getActiveChunks();
    // TODO(Ben): Race condition!
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
                // Dispose the mesh and disconnect neighbors
                disposeChunkMesh(chunk);
                chunk->m_inLoadRange = false;
                ChunkHandle(chunk).release();
                disconnects.push_back(chunk);
            }
        } else {
            // Check for connecting neighbors
            if (!chunk->m_inLoadRange) {
                chunk->m_inLoadRange = true;
                connects.push_back(chunk);
                ChunkHandle(chunk).acquire();
            }
            // Check for generation
            if (doGen) {
                if (chunk->pendingGenLevel != GEN_DONE) {
                    // Submit a generation query
                 //   ChunkQuery* q = new ChunkQuery;
                 //   q->set(chunk->getChunkPosition().pos, GEN_DONE, true);
                //    m_cmp->chunkGrids[agentPosition.face].submitQuery(q);
                } else if (chunk->genLevel == GEN_DONE && chunk->needsRemesh()) {
                    // TODO(Ben): Get meshing outa here
                    requestChunkMesh(chunk);
                }
            }
        }
    }

    for (auto& h : connects) {
        grid.acquireNeighbors(h);
    }
    for (auto& h : disconnects) {
        grid.releaseNeighbors(h);
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
        chunk->hasCreatedMesh = false;
        chunk->remeshFlags |= 1;
    }
}

ChunkMeshTask* SphericalVoxelComponentUpdater::trySetMeshDependencies(ChunkHandle chunk) {
    return false;
    // TODO(Ben): This could be optimized a bit
    ChunkHandle& left = chunk->left;
    ChunkHandle& right = chunk->right;
    ChunkHandle& bottom = chunk->bottom;
    ChunkHandle& top = chunk->top;
    ChunkHandle& back = chunk->back;
    ChunkHandle& front = chunk->front;

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
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT] = left.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT] = right.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_FRONT] = chunk->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_BACK] = chunk->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_TOP] = chunk->top.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_BOT] = chunk->bottom.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_BACK] = left->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_FRONT] = left->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_TOP] = leftTop.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_BOT] = leftBot.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_TOP_BACK] = leftTop->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_TOP_FRONT] = leftTop->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_BOT_BACK] = leftBot->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_BOT_FRONT] = leftBot->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_BACK] = right->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_FRONT] = right->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_TOP] = rightTop.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_BOT] = rightBot.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_TOP_BACK] = rightTop->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_TOP_FRONT] = rightTop->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_BOT_BACK] = rightBot->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_BOT_FRONT] = rightBot->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_TOP_BACK] = top->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_TOP_FRONT] = top->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_BOT_BACK] = bottom->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_BOT_FRONT] = bottom->front.acquire();

    return meshTask;
}
