#include "stdafx.h"
#include "SpaceSystemUpdater.h"

#include "SoAState.h"

#include <Vorb/Timing.h>

void SpaceSystemUpdater::init(const SoaState* soaState) {
    // Set planet rotation
    m_axisRotationComponentUpdater.update(soaState->spaceSystem, soaState->time);
    // Set initial position
    m_orbitComponentUpdater.update(soaState->spaceSystem, soaState->time);
}

void SpaceSystemUpdater::update(SoaState* soaState, const f64v3& spacePos, const f64v3& voxelPos) {

    // Get handles
    SpaceSystem* spaceSystem = soaState->spaceSystem;
    // const GameSystem* gameSystem = soaState->gameSystem;

    // Update planet rotation
    m_axisRotationComponentUpdater.update(spaceSystem, soaState->time);

    // Update far terrain
    // Update this BEFORE sphericalTerrain
    m_farTerrainComponentUpdater.update(spaceSystem, voxelPos * KM_PER_VOXEL);

    // Update Spherical Terrain
    m_sphericalTerrainComponentUpdater.update(soaState, spacePos);

    // Update voxels
    m_sphericalVoxelComponentUpdater.update(soaState);

    // Update Orbits ( Do this last)
    m_orbitComponentUpdater.update(spaceSystem, soaState->time);
}

void SpaceSystemUpdater::glUpdate(const SoaState* soaState) {
    m_sphericalTerrainComponentUpdater.glUpdate(soaState);
    m_farTerrainComponentUpdater.glUpdate(soaState->spaceSystem);
}