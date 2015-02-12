///
/// FarTerrainPatch.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines a class for a patch of far-terrain, not to be confused
/// with spherical terrain.
///

#pragma once

#ifndef FarTerrainPatch_h__
#define FarTerrainPatch_h__

#include "SphericalTerrainPatch.h"


class FarTerrainPatch {
    FarTerrainPatch() {};
    ~FarTerrainPatch();

    /// Initializes the patch
    /// @param gridPosition: Position on the 2d face grid
    /// @param sphericalTerrainData: Shared data
    /// @param width: Width of the patch in KM
    void init(const f64v2& gridPosition,
              WorldCubeFace cubeFace,
              int lod,
              const SphericalTerrainData* sphericalTerrainData,
              f64 width,
              TerrainRpcDispatcher* dispatcher);

    /// Updates the patch
    /// @param cameraPos: Position of the camera
    void update(const f64v3& cameraPos);

    /// Frees resources
    void destroy();

    /// @return true if it has a generated mesh
    bool hasMesh() const { return (m_mesh && m_mesh->m_isRenderable); }

    /// @return true if it has a mesh, or all of its children are
    /// renderable.
    bool isRenderable() const;

    /// Checks if the point is over the horizon
    /// @param relCamPos: Relative observer position
    /// @param point: The point to check
    /// @param planetRadius: Radius of the planet
    static bool isOverHorizon(const f32v3 &relCamPos, const f32v3 &point, f32 planetRadius);
    static bool isOverHorizon(const f64v3 &relCamPos, const f64v3 &point, f64 planetRadius);

    static const int INDICES_PER_QUAD = 6;
    static const int INDICES_PER_PATCH = (PATCH_WIDTH - 1) * (PATCH_WIDTH + 3) * INDICES_PER_QUAD;
private:
    void requestMesh();

    f64v2 m_gridPosition = f64v2(0.0); ///< Position on 2D grid
    f64v3 m_worldPosition = f64v3(0.0); ///< Position relative to world
    f64 m_distance = 1000000000.0; ///< Distance from camera
    int m_lod = 0; ///< Level of detail
    WorldCubeFace m_cubeFace; ///< Which cube face grid it is on

    f64 m_width = 0.0; ///< Width of the patch in KM

    TerrainRpcDispatcher* m_dispatcher = nullptr;
    SphericalTerrainMesh* m_mesh = nullptr;

    const SphericalTerrainData* m_sphericalTerrainData = nullptr; ///< Shared data pointer
    FarTerrainPatch* m_children = nullptr; ///< Pointer to array of 4 children
};

#endif // FarTerrainPatch_h__
