#include "stdafx.h"
#include "SoaController.h"

#include <Vorb/ecs/Entity.h>

#include "App.h"
#include "GameSystemAssemblages.h"
#include "GameSystemUpdater.h"
#include "SoaState.h"
#include "Options.h"
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

    GameSystem* gameSystem = state->gameSystem.get();
    SpaceSystem* spaceSystem = state->spaceSystem.get();

    if (state->isNewGame) {

        auto& svcmp = spaceSystem->m_sphericalVoxelCT.getFromEntity(state->startingPlanet);
        auto& arcmp = spaceSystem->m_axisRotationCT.getFromEntity(state->startingPlanet);
        auto& npcmp = spaceSystem->m_namePositionCT.getFromEntity(state->startingPlanet);

        // Hacky way to Set initial velocity based on the planet
        /*OrbitComponentUpdater updater;
        f64 nextTime = state->time + state->timeStep * 60.0;
        updater.update(state->spaceSystem.get(), nextTime);
        f64v3 nextPos = npcmp.position;
        updater.update(state->spaceSystem.get(), state->time);
        f64v3 velocity = (nextPos - npcmp.position) / 60.0;*/

        f64v3 velocity(0.0);
        // Create the player entity
        state->playerEntity = GameSystemAssemblages::createPlayer(state->gameSystem.get(), state->startSpacePos,
                                                                  f64q(), 73.0f,
                                                                  velocity, graphicsOptions.fov, m_app->getWindow().getAspectRatio());

        auto& vpcmp = gameSystem->voxelPosition.getFromEntity(state->playerEntity);
        auto& spcmp = gameSystem->spacePosition.getFromEntity(state->playerEntity);

        f64v3 spacePos = state->startSpacePos;

        spcmp.position = arcmp.currentOrientation * spacePos + glm::normalize(arcmp.currentOrientation * spacePos) * 360.0 + npcmp.position;
        GameSystemUpdater::updateVoxelPlanetTransitions(gameSystem, spaceSystem, state);
    } else {
        // TODO(Ben): This
    }
}
