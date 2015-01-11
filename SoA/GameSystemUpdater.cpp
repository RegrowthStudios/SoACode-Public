#include "stdafx.h"
#include "GameSystemUpdater.h"

#include "GameSystem.h"

void GameSystemUpdater::update(OUT GameSystem* gameSystem) {
    // Update entity tables
    updatePhysics(gameSystem);
    updateCollision(gameSystem);
    updateMoveInput(gameSystem);

    // Update voxel planet transitions every 60 frames
    m_frameCounter++;
    if (m_frameCounter == 60) {
        updateVoxelPlanetTransitions(gameSystem);
    }
}

void GameSystemUpdater::updateVoxelPlanetTransitions(OUT GameSystem* gameSystem) {

    m_frameCounter = 0;
}

void GameSystemUpdater::updatePhysics(OUT GameSystem* gameSystem) {
    for (auto& it : gameSystem->physicsCT) {

    }
}

void GameSystemUpdater::updateCollision(OUT GameSystem* gameSystem) {
    for (auto& it : gameSystem->aabbCollidableCT) {

    }
}

void GameSystemUpdater::updateMoveInput(OUT GameSystem* gameSystem) {
    for (auto& it : gameSystem->moveInputCT) {

    }
}
