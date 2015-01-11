#include "stdafx.h"
#include "PhysicsComponentUpdater.h"

#include "GameSystem.h"
#include "SpaceSystem.h"

// TODO(Ben): Timestep
void PhysicsComponentUpdater::update(OUT GameSystem* gameSystem, const SpaceSystem* spaceSystem) {
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