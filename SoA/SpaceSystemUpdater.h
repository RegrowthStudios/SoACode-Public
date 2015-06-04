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

#include "AxisRotationComponentUpdater.h"
#include "FarTerrainComponentUpdater.h"
#include "OrbitComponentUpdater.h"
#include "SphericalTerrainComponentUpdater.h"
#include "SphericalVoxelComponentUpdater.h"

class SpaceSystem;
class GameSystem;
struct SoaState;

class SpaceSystemUpdater {
public:
    void init(const SoaState* soaState);
    void update(const SoaState* soaState, const f64v3& spacePos, const f64v3& voxelPos);

    /// Updates OpenGL specific stuff, should be called on render thread
    void glUpdate(const SoaState* soaState);

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
    FarTerrainComponentUpdater m_farTerrainComponentUpdater;
};

#endif // SpaceSystemUpdater_h__
