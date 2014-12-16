///
/// SphericalTerrainComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 3 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines the component for creating spherical terrain
/// for planets and stuff.
///

#pragma once

#ifndef SphericalTerrainComponent_h__
#define SphericalTerrainComponent_h__

#include "stdafx.h"
#include "SphericalTerrainPatch.h"

#include <deque>

class NamePositionComponent;
class Camera;

class SphericalTerrainComponent {
public:
    /// Initialize the spherical terrain
    /// @param radius: Radius of the planet, must be multiple of 32.
    void init(f64 radius);

    void update(const f64v3& cameraPos,
                const NamePositionComponent* npComponent);

    void draw(const Camera* camera,
              vg::GLProgram* terrainProgram,
              const NamePositionComponent* npComponent);
private:
    void initPatches();

    SphericalTerrainPatch* m_patches = nullptr; ///< Buffer for top level patches
    SphericalTerrainData* m_sphericalTerrainData = nullptr;
};

#endif // SphericalTerrainComponent_h__