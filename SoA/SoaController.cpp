#include "stdafx.h"
#include "SoaController.h"

#include <Vorb/ecs/Entity.h>

#include "App.h"
#include "GameSystemAssemblages.h"
#include "GameSystemUpdater.h"
#include "SoaState.h"
#include "SoaOptions.h"
#include "SoaEngine.h"
#include "OrbitComponentUpdater.h"

SoaController::~SoaController() {
    // Empty
}

void SoaController::startGame(SoaState* state) {
    // Load game ECS
    SoaEngine::loadGameSystem(state);

    GameSystem* gameSystem = state->gameSystem;
    SpaceSystem* spaceSystem = state->spaceSystem;

    if (state->isNewGame) {
        // Create the player entity and make the initial planet his parent
        if (state->startingPlanet) {
            state->playerEntity = GameSystemAssemblages::createPlayer(state->gameSystem, state->startSpacePos,
                                                                      f64q(), 73.0f,
                                                                      f64v3(0.0),
                                                                      state->startingPlanet,
                                                                      spaceSystem->sphericalGravity.getComponentID(state->startingPlanet),
                                                                      spaceSystem->sphericalTerrain.getComponentID(state->startingPlanet));
        } else {
            state->playerEntity = GameSystemAssemblages::createPlayer(state->gameSystem, state->startSpacePos,
                                                                      f64q(), 73.0f,
                                                                      f64v3(0.0),
                                                                      0, 0, 0);
        }
    } else {
        // TODO(Ben): This
    }
}
