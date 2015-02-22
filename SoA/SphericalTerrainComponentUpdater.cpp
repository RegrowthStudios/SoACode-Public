#include "stdafx.h"
#include "SphericalTerrainComponentUpdater.h"

#include "SoaState.h"
#include "SpaceSystem.h"
#include "SpaceSystemAssemblages.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainCpuGenerator.h"
#include "SphericalTerrainGpuGenerator.h"
#include "VoxelCoordinateSpaces.h"

void SphericalTerrainComponentUpdater::update(const SoaState* state, const f64v3& cameraPos) {

    SpaceSystem* spaceSystem = state->spaceSystem.get();
    for (auto& it : spaceSystem->m_sphericalTerrainCT) {
      
        SphericalTerrainComponent& stCmp = it.second;
        const NamePositionComponent& npComponent = spaceSystem->m_namePositionCT.getFromEntity(it.first);
        const AxisRotationComponent& arComponent = spaceSystem->m_axisRotationCT.getFromEntity(it.first);
        /// Calculate camera distance
        f64v3 relativeCameraPos = arComponent.invCurrentOrientation * (cameraPos - npComponent.position);
        f64 distance = glm::length(relativeCameraPos);

        // If inactive, force patch deletion
        if (!it.second.active) distance = LOAD_DIST * 10.0;

        if (distance <= LOAD_DIST) {
            // In range, allocate if needed
            if (!stCmp.patches) {
                initPatches(stCmp);
            }

            // Update patches
            for (int i = 0; i < ST_TOTAL_PATCHES; i++) {
                stCmp.patches[i].update(relativeCameraPos);
            }
        } else {
            // Out of range, delete everything
            if (stCmp.patches) {
                delete[] stCmp.patches;
                stCmp.patches = nullptr;
            }
        }

        // Handle voxel component
        if (stCmp.needsVoxelComponent) {
            // Check for creating a new component
            if (!stCmp.sphericalVoxelComponent) {
                // TODO(Ben): FarTerrain should be clientSide only
                // Add far terrain component (CLIENT SIDE)
                stCmp.farTerrainComponent = SpaceSystemAssemblages::addFarTerrainComponent(spaceSystem, it.first, stCmp,
                                                                                           stCmp.startVoxelPosition.face);
                // Add spherical voxel component (SERVER SIDE)
                stCmp.sphericalVoxelComponent = SpaceSystemAssemblages::addSphericalVoxelComponent(spaceSystem, it.first,
                                                                                                   spaceSystem->m_sphericalTerrainCT.getComponentID(it.first),
                                                                                                   stCmp.farTerrainComponent,
                                                                                                   stCmp.axisRotationComponent,
                                                                                                   stCmp.namePositionComponent,
                                                                                                   stCmp.startVoxelPosition, state);
            }
        } else {
            // Check for deleting components
            // TODO(Ben): We need to do refcounting for MP!
            if (stCmp.sphericalVoxelComponent) {
                SpaceSystemAssemblages::removeSphericalVoxelComponent(spaceSystem, it.first);
                SpaceSystemAssemblages::removeFarTerrainComponent(spaceSystem, it.first);
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
    const f64& patchWidth = cmp.sphericalTerrainData->patchWidth;

    // Allocate top level patches
    cmp.patches = new TerrainPatch[ST_TOTAL_PATCHES];

    int center = ST_PATCH_ROW / 2;
    f64v2 gridPos;
    int index = 0;

    // Init all the top level patches for each of the 6 grids
    for (int face = 0; face < NUM_FACES; face++) {
        for (int z = 0; z < ST_PATCH_ROW; z++) {
            for (int x = 0; x < ST_PATCH_ROW; x++) {
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
