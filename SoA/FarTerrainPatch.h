///
/// FarTerrainPatch.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines a class for a patch of far-terrain
///

#pragma once

#ifndef FarTerrainPatch_h__
#define FarTerrainPatch_h__

#include "SphericalTerrainPatch.h"

class FarTerrainPatch : public SphericalTerrainPatch {
public:
    FarTerrainPatch() {};
    ~FarTerrainPatch();

    /// Initializes the patch
    /// @param gridPosition: Position on the 2d face grid
    /// @param sphericalTerrainData: Shared data
    /// @param width: Width of the patch in KM
    virtual void init(const f64v2& gridPosition,
              WorldCubeFace cubeFace,
              int lod,
              const SphericalTerrainData* sphericalTerrainData,
              f64 width,
              TerrainRpcDispatcher* dispatcher) override;

    /// Updates the patch
    /// @param cameraPos: Position of the camera
    virtual void update(const f64v3& cameraPos) override;
protected:
    /// Requests a mesh via RPC
    virtual void requestMesh() override;
};

#endif // FarTerrainPatch_h__
