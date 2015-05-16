///
/// TerrainPatch.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 15 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// A terrain patch for use with a SphericalTerrainComponent
///

#pragma once

#ifndef TerrainPatch_h__
#define TerrainPatch_h__

#include "TerrainGenerator.h"
#include "VoxelCoordinateSpaces.h"
#include "TerrainPatchConstants.h"

#include <Vorb/graphics/gtypes.h>

class TerrainRpcDispatcher;
class TerrainGenDelegate;
class TerrainPatchMesh;

// Shared data for terrain patches
struct TerrainPatchData { // TODO(Ben): probably dont need this
    friend struct SphericalTerrainComponent;

    TerrainPatchData(f64 radius, f64 patchWidth) :
        radius(radius),
        patchWidth(patchWidth) {
        // Empty
    }

    f64 radius; ///< Radius of the planet in KM
    f64 patchWidth; ///< Width of a patch in KM
};

// TODO(Ben): Sorting, Atmosphere, Frustum Culling
// fix redundant quality changes
class TerrainPatch {
public:
    TerrainPatch() { };
    virtual ~TerrainPatch();
    
    /// Initializes the patch
    /// @param gridPosition: Position on the 2d face grid
    /// @param sphericalTerrainData: Shared data
    /// @param width: Width of the patch in KM
    virtual void init(const f64v2& gridPosition,
              WorldCubeFace cubeFace,
              int lod,
              const TerrainPatchData* sphericalTerrainData,
              f64 width,
              TerrainRpcDispatcher* dispatcher);

    /// Updates the patch
    /// @param cameraPos: Position of the camera
    virtual void update(const f64v3& cameraPos);

    /// Frees resources
    void destroy();

    /// @return true if it has a generated mesh
    bool hasMesh() const;

    /// @return true if it has a mesh, or all of its children are
    /// renderable.
    bool isRenderable() const;

                            
    static bool isOverHorizon(const f64v3 &relCamPos, const f64v3 &point, f64 planetRadius);

    static void setQuality(int quality);

    /// Returns true if the patch can subdivide
    bool canSubdivide() const;
protected:
    /// Requests a mesh via RPC
    virtual void requestMesh();
    /// Calculates the closest point to the camera, as well as distance
    /// @param cameraPos: position of the observer
    /// @return closest point on the AABB
    f64v3 calculateClosestPointAndDist(const f64v3& cameraPos);

    static f32 DIST_MIN;
    static f32 DIST_MAX;
    static f32 MIN_SIZE;
    static int PATCH_MAX_LOD;

    f64v2 m_gridPos = f64v2(0.0); ///< Position on 2D grid
    f64v3 m_aabbPos = f64v3(0.0); ///< Position relative to world
    f64v3 m_aabbDims = f64v3(0.0);
    f64 m_distance = 1000000000.0; ///< Distance from camera
    int m_lod = 0; ///< Level of detail
    WorldCubeFace m_cubeFace; ///< Which cube face grid it is on

    f64 m_width = 0.0; ///< Width of the patch in KM

    TerrainRpcDispatcher* m_dispatcher = nullptr;
    TerrainPatchMesh* m_mesh = nullptr;

    const TerrainPatchData* m_terrainPatchData = nullptr; ///< Shared data pointer
    TerrainPatch* m_children = nullptr; ///< Pointer to array of 4 children
};

#endif // TerrainPatch_h__