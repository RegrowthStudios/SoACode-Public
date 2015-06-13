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
        ChunkQuery* q = new ChunkQuery;
        q->set(chunkPosition, GEN_DONE, true);
        m_cmp->chunkGrids[agentPosition.face].submitQuery(q);
    }

    updateChunks(m_cmp->chunkGrids[agentPosition.face], agentPosition);

    // TODO(Ben): This doesn't scale for multiple agents
    m_cmp->chunkGrids[agentPosition.face].update();
}

void SphericalVoxelComponentUpdater::updateChunks(NChunkGrid& grid, const VoxelPosition3D& agentPosition) {
    // Get render distance squared
    f32 renderDist2 = (soaOptions.get(OPT_VOXEL_RENDER_DISTANCE).value.f + (f32)CHUNK_WIDTH);
    renderDist2 *= renderDist2;

    // Loop through all currently active chunks
    NChunk* chunk = grid.getActiveChunks();
    while (chunk != nullptr) {

        // Calculate distance TODO(Ben): Maybe don't calculate this every frame? Or use sphere approx?
        chunk->m_distance2 = computeDistance2FromChunk(chunk->getVoxelPosition().pos, agentPosition.pos);

        // Check container update
        if (chunk->genLevel == GEN_DONE) {
            chunk->updateContainers();
        }

        // Check for unload
        if (chunk->m_distance2 > renderDist2 && chunk->refCount == 0) {
            // Unload the chunk
            NChunk* tmp = chunk;
            chunk = chunk->getNextActive();
            grid.removeChunk(tmp);
        } else {
            // Check for neighbor loading TODO(Ben): Don't keep redundantly checking edges? Distance threshold?
            if (!chunk->hasAllNeighbors() && chunk->genLevel > GEN_TERRAIN) {
                tryLoadChunkNeighbors(chunk, agentPosition, renderDist2);
            }
            chunk = chunk->getNextActive();
        }
    }
}

void SphericalVoxelComponentUpdater::tryLoadChunkNeighbors(NChunk* chunk, const VoxelPosition3D& agentPosition, f32 loadDist2) {
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