#include "stdafx.h"
#include "SpaceSystemUpdater.h"

#include "SoAState.h"

void SpaceSystemUpdater::update(OUT SpaceSystem* spaceSystem, const GameSystem* gameSystem,
                                const SoaState* soaState, const f64v3& spacePos) {

    // Update planet rotation
    m_axisRotationComponentUpdater.update(spaceSystem, soaState->time);

    // Update Spherical Terrain
    m_sphericalTerrainComponentUpdater.update(spaceSystem, spacePos);

    // Update far terrain
    m_farTerrainComponentUpdater.update(spaceSystem, spacePos);

    // Update voxels
    m_sphericalVoxelComponentUpdater.update(spaceSystem, gameSystem, soaState);

    // Update Orbits ( Do this last)
    m_orbitComponentUpdater.update(spaceSystem, soaState->time);
}

void SpaceSystemUpdater::glUpdate(OUT SpaceSystem* spaceSystem) {
    m_sphericalTerrainComponentUpdater.glUpdate(spaceSystem);
}