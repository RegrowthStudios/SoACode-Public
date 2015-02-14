#include "stdafx.h"
#include "FarTerrainComponentUpdater.h"


#include "SpaceSystem.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainGpuGenerator.h"
#include "SphericalTerrainCpuGenerator.h"
#include "VoxelCoordinateSpaces.h"
#include "FarTerrainPatch.h"

const int SINGLE_FACE_PATCHES = TOTAL_PATCHES / NUM_FACES;
#define KM_PER_VOXEL 0.0005

void FarTerrainComponentUpdater::update(SpaceSystem* spaceSystem, const f64v3& cameraPos) {
    for (auto& it : spaceSystem->m_farTerrainCT) {
        FarTerrainComponent& cmp = it.second;
        /// Calculate camera distance
        f64v3 relativeCameraPos = cameraPos * KM_PER_VOXEL;
        f64 distance = glm::length(relativeCameraPos);

        if (distance <= LOAD_DIST) {
            // In range, allocate if needed
            if (!cmp.patches) {
                initPatches(cmp);
            }

            // Update patches
            for (int i = 0; i < SINGLE_FACE_PATCHES; i++) {
                cmp.patches[i].update(relativeCameraPos);
            }
        } else {
            // Out of range, delete everything
            if (cmp.patches) {
                delete[] cmp.patches;
                cmp.patches = nullptr;
            }
        }
    }
}

void FarTerrainComponentUpdater::glUpdate(SpaceSystem* spaceSystem) {
    for (auto& it : spaceSystem->m_farTerrainCT) {
        it.second.gpuGenerator->update();
    }
}

void FarTerrainComponentUpdater::initPatches(FarTerrainComponent& cmp) {
    const f64& patchWidth = cmp.sphericalTerrainData->getPatchWidth();

    // Allocate top level patches
    cmp.patches = new FarTerrainPatch[SINGLE_FACE_PATCHES];

    int center = PATCH_ROW / 2;
    f64v2 gridPos;
    int index = 0;

    // Init all the top level patches for each of the 6 grids

    for (int z = 0; z < PATCH_ROW; z++) {
        for (int x = 0; x < PATCH_ROW; x++) {
            auto& p = cmp.patches[index++];
            gridPos.x = (x - center) * patchWidth;
            gridPos.y = (z - center) * patchWidth;
            p.init(gridPos, static_cast<WorldCubeFace>(cmp.face),
                   0, cmp.sphericalTerrainData, patchWidth,
                   cmp.rpcDispatcher);
        }
    }
    
}
