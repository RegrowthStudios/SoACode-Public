#include "stdafx.h"
#include "PhysicsComponentUpdater.h"

#include "GameSystem.h"
#include "SpaceSystem.h"

// TODO(Ben): Timestep
void PhysicsComponentUpdater::update(OUT GameSystem* gameSystem, const SpaceSystem* spaceSystem) {
#define VOXELS_PER_KM 2000.0
    for (auto& it : gameSystem->physicsCT) {
        auto& cmp = it.second;
        // Get the position component
        auto& spcmp = gameSystem->spacePositionCT.get(cmp.spacePositionComponent);
        // Voxel position dictates space position
        if (cmp.voxelPositionComponent) {
            // NOTE: This is costly

            auto& vpcmp = gameSystem->voxelPositionCT.get(cmp.voxelPositionComponent);
            vpcmp.position += cmp.velocity * VOXELS_PER_KM; // * VOXELS_PER_KM is temporary

            // Compute the space position and orientation from voxel position and orientation
            auto& svcmp = spaceSystem->m_sphericalVoxelCT.get(vpcmp.parentVoxelComponent);
            auto& npcmp = spaceSystem->m_namePositionCT.get(svcmp.namePositionComponent);
            auto& arcmp = spaceSystem->m_axisRotationCT.get(svcmp.axisRotationComponent);

            vpcmp.mapData.ipos = vpcmp.position.z / CHUNK_WIDTH;
            vpcmp.mapData.jpos = vpcmp.position.x / CHUNK_WIDTH;

            f64v3 spacePos = vpcmp.mapData.getWorldNormal(svcmp.voxelRadius) * (vpcmp.position.y + svcmp.voxelRadius) / VOXELS_PER_KM;

            spcmp.position = arcmp.currentOrientation * spacePos + npcmp.position;
            // TODO(Ben): This is expensive as fuck. Make sure you only do this for components that actually need it
            spcmp.orientation = (vpcmp.mapData.calculateVoxelToSpaceQuat(vpcmp.position, svcmp.voxelRadius)) * arcmp.currentOrientation * vpcmp.orientation;
        } else {
            spcmp.position += cmp.velocity; // * timestep
        }
        printVec("POS :", spcmp.position);
    }
}