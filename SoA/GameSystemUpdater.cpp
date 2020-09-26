#include "stdafx.h"
#include "GameSystemUpdater.h"

#include "FreeMoveComponentUpdater.h"
#include "GameSystem.h"
#include "GameSystemAssemblages.h"
#include "Inputs.h"
#include "SoAState.h"
#include "SpaceSystem.h"
#include "SpaceSystemAssemblages.h"
#include "TerrainPatch.h"
#include "VoxelSpaceConversions.h"
#include "VoxelSpaceUtils.h"

#include <Vorb/utils.h>

GameSystemUpdater::GameSystemUpdater(OUT SoaState* soaState, InputMapper* inputMapper) :
    m_soaState(soaState),
    m_inputMapper(inputMapper) {
}

GameSystemUpdater::~GameSystemUpdater() {
    
}

void GameSystemUpdater::update(OUT GameSystem* gameSystem, OUT SpaceSystem* spaceSystem, const SoaState* soaState VORB_MAYBE_UNUSED) {
    // Update component tables
    m_freeMoveUpdater.update(gameSystem, spaceSystem);
    m_headUpdater.update(gameSystem);
    m_aabbCollidableUpdater.update(gameSystem, spaceSystem);
    m_parkourUpdater.update(gameSystem, spaceSystem, soaState);
    m_physicsUpdater.update(gameSystem, spaceSystem);
    m_collisionUpdater.update(gameSystem);
    m_chunkSphereUpdater.update(gameSystem, spaceSystem);
    m_frustumUpdater.update(gameSystem);
}
