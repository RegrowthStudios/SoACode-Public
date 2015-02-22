#include "stdafx.h"
#include "PhysicsComponentUpdater.h"

#include "GameSystem.h"
#include "SpaceSystem.h"
#include "VoxelSpaceConversions.h"
#include "VoxelSpaceUtils.h"
#include "global.h"

// TODO(Ben): Timestep
void PhysicsComponentUpdater::update(OUT GameSystem* gameSystem, SpaceSystem* spaceSystem) {
    
    // TODO(Ben): These are temporary for gravity
#define FPS 60.0

    for (auto& it : gameSystem->physics) {
        auto& cmp = it.second;
        // Get the position component
        auto& spcmp = gameSystem->spacePosition.get(cmp.spacePositionComponent);

        // Voxel position dictates space position
        if (cmp.voxelPositionComponent) {
            // NOTE: This is costly
            auto& vpcmp = gameSystem->voxelPosition.get(cmp.voxelPositionComponent);
            auto& svcmp = spaceSystem->m_sphericalVoxelCT.get(vpcmp.parentVoxelComponent);
            auto& npcmp = spaceSystem->m_namePositionCT.get(svcmp.namePositionComponent);
            auto& arcmp = spaceSystem->m_axisRotationCT.get(svcmp.axisRotationComponent);
            // Apply gravity
            if (spcmp.parentGravityId) {
                auto& gravCmp = spaceSystem->m_sphericalGravityCT.get(spcmp.parentGravityId);
                f64 height = (vpcmp.gridPosition.pos.y + svcmp.voxelRadius) * M_PER_VOXEL;
                f64 fgrav = M_G * gravCmp.mass / (height * height);
                // We don't account mass since we only calculate force on the object
                cmp.velocity.y -= (fgrav / M_PER_KM) / FPS;
            }

            vpcmp.gridPosition.pos += cmp.velocity;
            f64v3 spacePos = VoxelSpaceConversions::voxelToWorld(vpcmp.gridPosition, svcmp.voxelRadius) * KM_PER_VOXEL;
           
            // Compute the relative space position and orientation from voxel position and orientation
            spcmp.position = arcmp.currentOrientation * spacePos;
            // TODO(Ben): This is expensive as fuck. Make sure you only do this for components that actually need it
            spcmp.orientation = arcmp.currentOrientation * VoxelSpaceUtils::calculateVoxelToSpaceQuat(vpcmp.gridPosition, svcmp.voxelRadius) * vpcmp.orientation;
        } else {
            // Apply gravity
            // TODO(Ben): Optimize and fix with timestep
            if (spcmp.parentGravityId) {
                auto& gravCmp = spaceSystem->m_sphericalGravityCT.get(spcmp.parentGravityId);
                // Because the player is the child of the parent, we just use relative position
                f64v3 distVec = -spcmp.position;
                f64 dist2 = glm::dot(distVec, distVec);
                distVec /= sqrt(dist2); // Normalize it
                f64 fgrav = M_G * gravCmp.mass / (dist2 * M_PER_KM * M_PER_KM);
                cmp.velocity += distVec * ((fgrav / M_PER_KM) / FPS);
            } else {
                // TODO(Ben): Use this?
               /* for (auto& sit : spaceSystem->m_sphericalGravityCT) {
                    const f64v3& pos = spaceSystem->m_namePositionCT.get(it.second.namePositionComponent).position;
                    f64v3 distVec = pos - spcmp.position;
                    f64 dist2 = glm::dot(distVec, distVec);
                    distVec /= sqrt(dist2); // Normalize it
                    f64 fgrav = M_G * it.second.mass / (dist2 * M_PER_KM * M_PER_KM);
                    cmp.velocity += distVec * (((fgrav / PLAYER_MASS) / M_PER_KM) / FPS);
                } */
            }
            
            spcmp.position += cmp.velocity; // * timestep
        }
    }
}