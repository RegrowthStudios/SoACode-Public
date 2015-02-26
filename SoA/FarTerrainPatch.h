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

#include "TerrainPatch.h"

// TODO(Ben): Linear fade to prevent LOD popping
class FarTerrainPatch : public TerrainPatch {
public:
    FarTerrainPatch() {};
    ~FarTerrainPatch();

    /// Initializes the patch
    /// @param gridPosition: Position on the 2d face grid
    /// @param sphericalTerrainData: Shared data
    /// @param width: Width of the patch in KM
    void init(const f64v2& gridPosition,
              WorldCubeFace cubeFace,
              int lod,
              const TerrainPatchData* sphericalTerrainData,
              f64 width,
              TerrainRpcDispatcher* dispatcher) override;

    /// Updates the patch
    /// @param cameraPos: Position of the camera
    void update(const f64v3& cameraPos) override;

    /// Checks if the point is over the horizon
    /// @param relCamPos: Relative observer position
    /// @param point: The point to check
    /// @param planetRadius: Radius of the planet
    static bool isOverHorizon(const f64v3 &relCamPos, const f64v3 &point, f64 planetRadius);
protected:
    /// Requests a mesh via RPC
    void requestMesh() override;
};

#endif // FarTerrainPatch_h__
