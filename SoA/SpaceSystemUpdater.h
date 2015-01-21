///
/// SpaceSystemUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 20 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Updates the SpaceSystem
///

#pragma once

#ifndef SpaceSystemUpdater_h__
#define SpaceSystemUpdater_h__

#include "OrbitComponentUpdater.h"
#include "SphericalVoxelComponentUpdater.h"
#include "SphericalTerrainComponentUpdater.h"
#include "AxisRotationComponentUpdater.h"
#include "SphericalVoxelComponentTable.h"

class SpaceSystem;
class GameSystem;
class SoaState;

class SpaceSystemUpdater {
public:
    void update(OUT SpaceSystem* spaceSystem, const GameSystem* gameSystem,
                const SoaState* soaState, const f64v3& spacePos);

    /// Updates OpenGL specific stuff, should be called on render thread
    void glUpdate(OUT SpaceSystem* spaceSystem);

private:
    /// Updaters
    friend class OrbitComponentUpdater;
    OrbitComponentUpdater m_orbitComponentUpdater;
    friend class SphericalVoxelComponentUpdater;
    SphericalVoxelComponentUpdater m_sphericalVoxelComponentUpdater;
    friend class SphericalTerrainComponentUpdater;
    SphericalTerrainComponentUpdater m_sphericalTerrainComponentUpdater;
    friend class AxisRotationComponentUpdater;
    AxisRotationComponentUpdater m_axisRotationComponentUpdater;
};

#endif // SpaceSystemUpdater_h__
