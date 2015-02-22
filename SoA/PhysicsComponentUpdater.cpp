#include "stdafx.h"
#include "PhysicsComponentUpdater.h"

#include "GameSystem.h"
#include "SpaceSystem.h"
#include "VoxelSpaceConversions.h"
#include "VoxelSpaceUtils.h"
#include "global.h"

// TODO(Ben): Timestep
void PhysicsComponentUpdater::update(OUT GameSystem* gameSystem, SpaceSystem* spaceSystem) {
    for (auto& it : gameSystem->physics) {
        auto& cmp = it.second;
        // Get the position component
        auto& spcmp = gameSystem->spacePosition.get(cmp.spacePositionComponent);

        // Voxel position dictates space position
        if (cmp.voxelPositionComponent) {
            // NOTE: This is costly

            auto& vpcmp = gameSystem->voxelPosition.get(cmp.voxelPositionComponent);
            vpcmp.gridPosition.pos += cmp.velocity;

            // Compute the space position and orientation from voxel position and orientation
            auto& svcmp = spaceSystem->m_sphericalVoxelCT.get(vpcmp.parentVoxelComponent);
            auto& npcmp = spaceSystem->m_namePositionCT.get(svcmp.namePositionComponent);
            auto& arcmp = spaceSystem->m_axisRotationCT.get(svcmp.axisRotationComponent);

            f64v3 spacePos = VoxelSpaceConversions::voxelToWorld(vpcmp.gridPosition, svcmp.voxelRadius) * KM_PER_VOXEL;

            spcmp.position = arcmp.currentOrientation * spacePos + npcmp.position;
            // TODO(Ben): This is expensive as fuck. Make sure you only do this for components that actually need it
            spcmp.orientation = arcmp.currentOrientation * VoxelSpaceUtils::calculateVoxelToSpaceQuat(vpcmp.gridPosition, svcmp.voxelRadius) * vpcmp.orientation;
        } else {
            // Apply gravity
            // TODO(Ben): Optimize and fix
#define PLAYER_MASS 80.0
#define FPS 60.0
            for (auto& it : spaceSystem->m_sphericalGravityCT) {
                const f64v3& pos = spaceSystem->m_namePositionCT.get(it.second.namePositionComponent).position;
                f64v3 distVec = pos - spcmp.position;
                f64 dist2 = glm::dot(distVec, distVec);
                distVec /= sqrt(dist2); // Normalize it
                f64 fgrav = M_G * it.second.mass / (dist2 * M_PER_KM * M_PER_KM);
                cmp.velocity += distVec * (((fgrav / PLAYER_MASS) / M_PER_KM) / FPS);
            }
            
            spcmp.position += cmp.velocity; // * timestep
        }
    }
}