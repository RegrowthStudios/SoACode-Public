#include "stdafx.h"
#include "GameSystemUpdater.h"

#include "GameSystem.h"

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
