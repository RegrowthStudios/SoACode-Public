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

    if (state->clientState.isNewGame) {
        // Create the player entity and make the initial planet his parent
        if (state->startingPlanet) {
            state->playerEntity = state->templateLib.build(*gameSystem, "Player");

            auto& spacePos = gameSystem->spacePosition.getFromEntity(state->playerEntity);
            spacePos.position = state->clientState.startSpacePos;
            spacePos.parentEntity = state->startingPlanet;
            spacePos.parentGravityID = spaceSystem->sphericalGravity.getComponentID(state->startingPlanet);
            spacePos.parentSphericalTerrainID = spaceSystem->sphericalTerrain.getComponentID(state->startingPlanet);
        
            auto& physics = gameSystem->physics.getFromEntity(state->playerEntity);
            physics.spacePositionComponent = gameSystem->spacePosition.getComponentID(state->playerEntity);
        } else {
            state->playerEntity = state->templateLib.build(*gameSystem, "Player");

            auto& spacePos = gameSystem->spacePosition.getFromEntity(state->playerEntity);
            spacePos.position = state->clientState.startSpacePos;
        }
    } else {
        // TODO(Ben): This
    }
}
