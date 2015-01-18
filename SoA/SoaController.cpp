#include "stdafx.h"
#include "SoaController.h"

#include <Vorb/ecs/Entity.h>

#include "GameSystemFactories.h"
#include "GameSystemUpdater.h"
#include "SoaState.h"

SoaController::~SoaController() {
    // Empty
}

void SoaController::startGame(OUT SoaState* state) {
    GameSystem& gameSystem = state->gameSystem;
    SpaceSystem& spaceSystem = state->spaceSystem;

    if (state->isNewGame) {
        // Create the player entity
        state->playerEntity = GameSystemFactories::createPlayer(&state->gameSystem, state->startSpacePos,
                                          f64q(), 73.0f, f64v3(0.0));
        
        auto& svcmp = spaceSystem.m_sphericalVoxelCT.getFromEntity(state->startingPlanet);
        auto& arcmp = spaceSystem.m_axisRotationCT.getFromEntity(state->startingPlanet);
        auto& npcmp = spaceSystem.m_namePositionCT.getFromEntity(state->startingPlanet);

        auto& vpcmp = gameSystem.voxelPositionCT.getFromEntity(state->playerEntity);
        auto& spcmp = gameSystem.spacePositionCT.getFromEntity(state->playerEntity);

        f64v3 spacePos = state->startSpacePos;

        spcmp.position = arcmp.currentOrientation * spacePos + npcmp.position;
        GameSystemUpdater::updateVoxelPlanetTransitions(&gameSystem, &spaceSystem, state);


    } else {

    }
}
