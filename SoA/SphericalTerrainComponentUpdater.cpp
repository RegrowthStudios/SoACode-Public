#include "stdafx.h"
#include "SphericalTerrainComponentUpdater.h"

#include "soaUtils.h"
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
        const NamePositionComponent& npComponent = spaceSystem->m_namePositionCT.get(stCmp.namePositionComponent);
        const AxisRotationComponent& arComponent = spaceSystem->m_axisRotationCT.get(stCmp.axisRotationComponent);
        /// Calculate camera distance
        f64v3 relativeCameraPos = arComponent.invCurrentOrientation * (cameraPos - npComponent.position);
        f64 distance = glm::length(relativeCameraPos);

        // Animation for fade
        if (stCmp.needsVoxelComponent) {
            stCmp.alpha -= TERRAIN_ALPHA_STEP;
            if (stCmp.alpha < 0.0f) {
                stCmp.alpha = 0.0f;
                // Force it to unload
                distance = LOAD_DIST * 10.0;
            }
        } else {
            stCmp.alpha += TERRAIN_ALPHA_STEP;
            if (stCmp.alpha > 1.0f) stCmp.alpha = 1.0f;
        }

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

        updateVoxelComponentLogic(state, it.first, stCmp);
    }
}

void SphericalTerrainComponentUpdater::glUpdate(SpaceSystem* spaceSystem) {
    for (auto& it : spaceSystem->m_sphericalTerrainCT) {
        if (it.second.alpha > 0.0f) it.second.gpuGenerator->update();
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

void SphericalTerrainComponentUpdater::updateVoxelComponentLogic(const SoaState* state, vecs::EntityID eid, SphericalTerrainComponent& stCmp) {
    SpaceSystem* spaceSystem = state->spaceSystem.get();
    // Handle voxel component
    if (stCmp.needsVoxelComponent) {
        // Check for creating a new component
        if (!stCmp.sphericalVoxelComponent) {
            // TODO(Ben): FarTerrain should be clientSide only
            // Add far terrain component (CLIENT SIDE)
            stCmp.farTerrainComponent = SpaceSystemAssemblages::addFarTerrainComponent(spaceSystem, eid, stCmp,
                                                                                       stCmp.startVoxelPosition.face);
            // Add spherical voxel component (SERVER SIDE)
            stCmp.sphericalVoxelComponent = SpaceSystemAssemblages::addSphericalVoxelComponent(spaceSystem, eid,
                                                                                               spaceSystem->m_sphericalTerrainCT.getComponentID(eid),
                                                                                               stCmp.farTerrainComponent,
                                                                                               stCmp.axisRotationComponent,
                                                                                               stCmp.namePositionComponent,
                                                                                               stCmp.startVoxelPosition.face,
                                                                                               state);
        }

        // Far terrain face transition
        if (stCmp.transitionFace != FACE_NONE) {
            static const f32 FACE_TRANS_DEC = 0.02f;
            if (stCmp.faceTransTime == START_FACE_TRANS) stCmp.needsFaceTransitionAnimation = true;
            stCmp.faceTransTime -= FACE_TRANS_DEC;
            if (stCmp.faceTransTime <= 0.0f) {
                // TODO(Ben): maybe tell voxels to switch to new face rather than just deleting
                //spaceSystem->m_sphericalVoxelCT.get(stCmp.sphericalVoxelComponent).transitionFace = stCmp.transitionFace;
                SpaceSystemAssemblages::removeSphericalVoxelComponent(spaceSystem, eid);
                // Add spherical voxel component (SERVER SIDE)
                stCmp.sphericalVoxelComponent = SpaceSystemAssemblages::addSphericalVoxelComponent(spaceSystem, eid,
                                                                                                   spaceSystem->m_sphericalTerrainCT.getComponentID(eid),
                                                                                                   stCmp.farTerrainComponent,
                                                                                                   stCmp.axisRotationComponent,
                                                                                                   stCmp.namePositionComponent,
                                                                                                   stCmp.transitionFace,
                                                                                                   state);
                // Reload the terrain
                auto& ftCmp = spaceSystem->m_farTerrainCT.get(stCmp.farTerrainComponent);
                ftCmp.transitionFace = stCmp.transitionFace;
                stCmp.transitionFace = FACE_NONE;
                stCmp.faceTransTime = 0.0f;
                std::cout << " RE-INIT Voxels\n";
            }
        } else if (!stCmp.needsFaceTransitionAnimation) {
            stCmp.faceTransTime = 1.0f;
        }
    } else {
        // Check for deleting components
        // TODO(Ben): We need to do refcounting for MP!
        if (stCmp.sphericalVoxelComponent) {
            // Mark far terrain for fadeout
            auto& ftCmp = spaceSystem->m_farTerrainCT.get(stCmp.farTerrainComponent);

            if (!ftCmp.shouldFade) {
                ftCmp.shouldFade = true;
                ftCmp.alpha = TERRAIN_DEC_START_ALPHA;
            } else if (ftCmp.alpha < 0.0f) {
                // We are faded out, so deallocate
                SpaceSystemAssemblages::removeSphericalVoxelComponent(spaceSystem, eid);
                SpaceSystemAssemblages::removeFarTerrainComponent(spaceSystem, eid);
                stCmp.sphericalVoxelComponent = 0;
                stCmp.farTerrainComponent = 0;
            }
        }
    }
}

