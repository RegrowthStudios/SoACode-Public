#include "stdafx.h"
#include "SphericalTerrainComponentUpdater.h"

#include "SpaceSystem.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainGpuGenerator.h"
#include "SphericalTerrainCpuGenerator.h"
#include "VoxelCoordinateSpaces.h"

void SphericalTerrainComponentUpdater::update(SpaceSystem* spaceSystem, const f64v3& cameraPos) {
    for (auto& it : spaceSystem->m_sphericalTerrainCT) {
      
        SphericalTerrainComponent& cmp = it.second;
        const NamePositionComponent& npComponent = spaceSystem->m_namePositionCT.getFromEntity(it.first);
        const AxisRotationComponent& arComponent = spaceSystem->m_axisRotationCT.getFromEntity(it.first);
        /// Calculate camera distance
        f64v3 relativeCameraPos = arComponent.invCurrentOrientation * (cameraPos - npComponent.position);
        f64 distance = glm::length(relativeCameraPos);

        // If inactive, force patch deletion
        if (!it.second.active) distance = LOAD_DIST * 10.0;

        if (distance <= LOAD_DIST) {
            // In range, allocate if needed
            if (!cmp.patches) {
                initPatches(cmp);
            }

            // Update patches
            for (int i = 0; i < TOTAL_PATCHES; i++) {
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

void SphericalTerrainComponentUpdater::glUpdate(SpaceSystem* spaceSystem) {
    for (auto& it : spaceSystem->m_sphericalTerrainCT) {
        if (it.second.active) it.second.gpuGenerator->update();
    }
}

void SphericalTerrainComponentUpdater::initPatches(SphericalTerrainComponent& cmp) {
    const f64& patchWidth = cmp.sphericalTerrainData->getPatchWidth();

    // Allocate top level patches
    cmp.patches = new SphericalTerrainPatch[TOTAL_PATCHES];

    int center = PATCH_ROW / 2;
    f64v2 gridPos;
    int index = 0;

    // Init all the top level patches for each of the 6 grids
    for (int face = 0; face < NUM_FACES; face++) {
        for (int z = 0; z < PATCH_ROW; z++) {
            for (int x = 0; x < PATCH_ROW; x++) {
                auto& p = cmp.patches[index++];
                gridPos.x = (x - center) * patchWidth;
                gridPos.y = (z - center) * patchWidth;
                p.init(gridPos, static_cast<WorldCubeFace>(face),
                       0, cmp.sphericalTerrainData, patchWidth,
                       cmp.rpcDispatcher);
            }
        }
    }
}
