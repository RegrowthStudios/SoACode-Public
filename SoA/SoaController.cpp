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

SoaController::SoaController(const App* app) :
    m_app(app) {
    // Empty
}

SoaController::~SoaController() {
    // Empty
}

void SoaController::startGame(OUT SoaState* state) {
    // Load game ECS
    SoaEngine::GameSystemLoadData loadData;
    SoaEngine::loadGameSystem(state, loadData);

    GameSystem* gameSystem = state->gameSystem;
    SpaceSystem* spaceSystem = state->spaceSystem;

    if (state->isNewGame) {
 
        auto& svcmp = spaceSystem->sphericalVoxel.getFromEntity(state->startingPlanet);
        auto& arcmp = spaceSystem->axisRotation.getFromEntity(state->startingPlanet);
        auto& npcmp = spaceSystem->namePosition.getFromEntity(state->startingPlanet);

        auto& np2 = spaceSystem->namePosition.get(spaceSystem->sphericalGravity.get(spaceSystem->sphericalGravity.getComponentID(state->startingPlanet)).namePositionComponent);

        // Create the player entity and make the initial planet his parent
        state->playerEntity = GameSystemAssemblages::createPlayer(state->gameSystem, state->startSpacePos,
                                                                  f64q(), 73.0f,
                                                                  f64v3(0.0), soaOptions.get(OPT_FOV).value.f, m_app->getWindow().getAspectRatio(),
                                                                  state->startingPlanet,
                                                                  spaceSystem->sphericalGravity.getComponentID(state->startingPlanet),
                                                                  spaceSystem->sphericalTerrain.getComponentID(state->startingPlanet));

        auto& spcmp = gameSystem->spacePosition.getFromEntity(state->playerEntity);

        const f64v3& spacePos = state->startSpacePos;

        spcmp.position = arcmp.currentOrientation * spacePos;
    } else {
        // TODO(Ben): This
    }
}
