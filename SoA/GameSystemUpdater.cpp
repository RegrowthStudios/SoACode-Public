#include "stdafx.h"
#include "GameSystemUpdater.h"

#include "FreeMoveComponentUpdater.h"
#include "GameSystem.h"
#include "GameSystemAssemblages.h"
#include "GameSystemEvents.h"
#include "Inputs.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "SpaceSystemAssemblages.h"
#include "TerrainPatch.h"
#include "VoxelSpaceConversions.h"
#include "VoxelSpaceUtils.h"

#include <Vorb/utils.h>
#include <Vorb/IntersectionUtils.inl>

GameSystemUpdater::GameSystemUpdater(OUT SoaState* soaState, InputMapper* inputMapper) :
    m_soaState(soaState),
    m_inputMapper(inputMapper) {

    m_events = std::make_unique<GameSystemEvents>(this);
}

GameSystemUpdater::~GameSystemUpdater() {
 
}

void GameSystemUpdater::update(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem, const SoaState* soaState) {

    // Update entity tables
    m_freeMoveUpdater.update(gameSystem, spaceSystem);
    m_physicsUpdater.update(gameSystem, spaceSystem);
    m_collisionUpdater.update(gameSystem);
    m_frustumUpdater.update(gameSystem);
}
