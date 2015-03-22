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

        auto& np2 = spaceSystem->m_namePositionCT.get(spaceSystem->m_sphericalGravityCT.get(spaceSystem->m_sphericalGravityCT.getComponentID(state->startingPlanet)).namePositionComponent);

        // Create the player entity and make the initial planet his parent
        state->playerEntity = GameSystemAssemblages::createPlayer(state->gameSystem.get(), state->startSpacePos,
                                                                  f64q(), 73.0f,
                                                                  f64v3(0.0), graphicsOptions.fov, m_app->getWindow().getAspectRatio(),
                                                                  state->startingPlanet,
                                                                  spaceSystem->m_sphericalGravityCT.getComponentID(state->startingPlanet),
                                                                  spaceSystem->m_sphericalTerrainCT.getComponentID(state->startingPlanet));

        auto& spcmp = gameSystem->spacePosition.getFromEntity(state->playerEntity);

        const f64v3& spacePos = state->startSpacePos;

        spcmp.position = arcmp.currentOrientation * spacePos + glm::normalize(arcmp.currentOrientation * spacePos) * 200.0;
    } else {
        // TODO(Ben): This
    }
}
