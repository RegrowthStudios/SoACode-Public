#include "stdafx.h"
#include "SphericalVoxelComponentUpdater.h"

#include <SDL2/SDL_timer.h> // For SDL_GetTicks

#include "Chunk.h"
#include "ChunkAllocator.h"
#include "ChunkGrid.h"
#include "ChunkIOManager.h"
#include "ChunkMeshManager.h"
#include "ChunkMeshTask.h"
#include "ChunkRenderer.h"
#include "ChunkUpdater.h"
#include "GameSystem.h"
#include "GenerateTask.h"
#include "PlanetGenData.h"
#include "SoaOptions.h"
#include "SoAState.h"
#include "SpaceSystem.h"
#include "SpaceSystemComponents.h"
#include "VoxelSpaceConversions.h"
#include "soaUtils.h"

#include <Vorb/voxel/VoxCommon.h>

void SphericalVoxelComponentUpdater::update(const SoaState* soaState) {
    SpaceSystem* spaceSystem = soaState->spaceSystem;
    GameSystem* gameSystem = soaState->gameSystem;
    if (spaceSystem->sphericalVoxel.getComponentListSize() > 1) {
        for (auto& it : spaceSystem->sphericalVoxel) {
            if (it.second.chunkGrids) {
                updateComponent(it.second);
            }
        }
    }
}

void SphericalVoxelComponentUpdater::updateComponent(SphericalVoxelComponent& cmp) {
    m_cmp = &cmp;
    // Update each world cube face
    for (int i = 0; i < 6; i++) {
        updateChunks(cmp.chunkGrids[i], true);
        cmp.chunkGrids[i].update();
    }
}

// TODO: Implement and remove VORB_UNUSED tags.
void SphericalVoxelComponentUpdater::updateChunks(ChunkGrid& grid VORB_UNUSED, bool doGen VORB_UNUSED) {
    // Get render distance squared
    f32 renderDist2 = (soaOptions.get(OPT_VOXEL_RENDER_DISTANCE).value.f + (f32)CHUNK_WIDTH);
    renderDist2 *= renderDist2;

}

