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
                updateComponent(playerPosCmp.gridPosition.pos);
            }
        }
    }
}

void SphericalVoxelComponentUpdater::updateComponent(const f64v3& position) {

 
}
