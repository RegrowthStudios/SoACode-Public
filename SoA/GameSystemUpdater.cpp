#include "stdafx.h"
#include "GameSystemUpdater.h"

#include "GameSystem.h"
#include "SpaceSystem.h"
#include "SphericalTerrainPatch.h"


void GameSystemUpdater::update(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem) {
    // Update entity tables
    updatePhysics(gameSystem, spaceSystem);
    updateCollision(gameSystem);
    updateMoveInput(gameSystem);

    // Update voxel planet transitions every 60 frames
    m_frameCounter++;
    if (m_frameCounter == 60) {
        updateVoxelPlanetTransitions(gameSystem, spaceSystem);
    }
}

void GameSystemUpdater::updateVoxelPlanetTransitions(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem) {
#define LOAD_DIST_MULT 1.05
    for (auto& it : gameSystem->spacePositionCT) {
        auto& spcmp = it.second;
        for (auto& sit : spaceSystem->m_sphericalTerrainCT) {
            auto& stcmp = sit.second;
            auto& npcmp = spaceSystem->m_namePositionCT.get(stcmp.namePositionComponent);
            // Calculate distance
            // TODO(Ben): Use ^2 distance as optimization to avoid sqrt
            f64 distance = glm::length(spcmp.position - npcmp.position);
            // Check for voxel transition
            if (distance < stcmp.sphericalTerrainData->getRadius() * LOAD_DIST_MULT && !spcmp.voxelPositionComponent) {
                // We need to transition to the voxels
                vcore::ComponentID vpid = gameSystem->voxelPositionCT.add(it.first);
                auto& vpcmp = gameSystem->voxelPositionCT.get(vpid);
                spcmp.voxelPositionComponent = vpid;
                // Calculate voxel position
                auto& rotcmp = spaceSystem->m_axisRotationCT.getFromEntity(sit.first);
                // TODO(Ben): This
            } else if (spcmp.voxelPositionComponent) {
                // We need to transition to space
                gameSystem->voxelPositionCT.remove(it.first);
                spcmp.voxelPositionComponent = 0;
                // TODO(Ben): Refcount SVC
            }
        }
    }
    m_frameCounter = 0;
}

// TODO(Ben): Timestep
void GameSystemUpdater::updatePhysics(OUT GameSystem* gameSystem, const SpaceSystem* spaceSystem) {
    for (auto& it : gameSystem->physicsCT) {
        auto& cmp = it.second;
        // Get the position component
        auto& spcmp = gameSystem->spacePositionCT.get(cmp.spacePositionComponent);
        // Voxel position dictates space position
        if (cmp.voxelPositionComponent) {
            auto& vpcmp = gameSystem->voxelPositionCT.get(cmp.voxelPositionComponent);
            vpcmp.position += cmp.velocity;
            // TODO(Ben): Compute spcmp.position from vpcmp.position
        } else {
            spcmp.position += cmp.velocity; // * timestep
        }
    }
}

void GameSystemUpdater::updateCollision(OUT GameSystem* gameSystem) {
    for (auto& it : gameSystem->aabbCollidableCT) {
        //TODO(Ben): this
    }
}

void GameSystemUpdater::updateMoveInput(OUT GameSystem* gameSystem) {
    for (auto& it : gameSystem->moveInputCT) {

    }
}
