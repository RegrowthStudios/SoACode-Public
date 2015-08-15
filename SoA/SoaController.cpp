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

    // TODO(Ben): Client only
    ClientState& clientState = state->clientState;
    if (clientState.isNewGame) {
        // Create the player entity and make the initial planet his parent
        if (clientState.startingPlanet) {
            clientState.playerEntity = state->templateLib.build(*gameSystem, "Player");

            auto& spacePos = gameSystem->spacePosition.getFromEntity(clientState.playerEntity);
            spacePos.position = clientState.startSpacePos;
            spacePos.parentEntity = clientState.startingPlanet;
            spacePos.parentGravityID = spaceSystem->sphericalGravity.getComponentID(clientState.startingPlanet);
            spacePos.parentSphericalTerrainID = spaceSystem->sphericalTerrain.getComponentID(clientState.startingPlanet);
        
            auto& physics = gameSystem->physics.getFromEntity(clientState.playerEntity);
            physics.spacePositionComponent = gameSystem->spacePosition.getComponentID(clientState.playerEntity);
        } else {
            clientState.playerEntity = state->templateLib.build(*gameSystem, "Player");

            auto& spacePos = gameSystem->spacePosition.getFromEntity(clientState.playerEntity);
            spacePos.position = state->clientState.startSpacePos;
        }
    } else {
        // TODO(Ben): This
    }
}
