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

class FarTerrainMesh {
public:
    friend class FarTerrainPatch;
    FarTerrainMesh(WorldCubeFace cubeFace) : m_cubeFace(cubeFace) {};
    ~FarTerrainMesh();

    /// Recycles the normal map
    /// @param recycler: Recycles the texture
    void recycleNormalMap(vg::TextureRecycler* recycler);

    /// Draws the terrain mesh
    /// @param cameraPos: Relative position of the camera
    /// @param camera: The camera
    /// @param program: Shader program for rendering
    void draw(const f64v3& relativePos, const Camera* camera,
              vg::GLProgram* program) const;

    /// Draws the water mesh
    /// @param relativePos: Relative position of the camera
    /// @param camera: The camera
    /// @param program: Shader program for rendering
    void drawWater(const f64v3& relativePos, const Camera* camera,
                   vg::GLProgram* program) const;

    /// Gets the point closest to the observer
    /// @param camPos: Position of observer
    /// @param point: Resulting point
    void getClosestPoint(const f32v3& camPos, OUT f32v3& point) const;
    void getClosestPoint(const f64v3& camPos, OUT f64v3& point) const;

private:
    VGVertexArray m_vao = 0; ///< Vertex array object
    VGVertexBuffer m_vbo = 0; ///< Vertex buffer object
    VGIndexBuffer m_ibo = 0; ///< Shared Index buffer object. DONT FREE THIS

    VGVertexBuffer m_wvbo = 0; ///< Water Vertex buffer object
    VGIndexBuffer m_wibo = 0; ///< Water Index Buffer Object

    f32v3 m_worldPosition = f32v3(0.0);
    f32v3 m_boundingBox = f32v3(0.0f); ///< AABB bounding box
    WorldCubeFace m_cubeFace;

    VGTexture m_normalMap = 0;
    int m_waterIndexCount = 0;

    volatile bool m_shouldDelete = false; ///< True when the mesh should be deleted
    bool m_isRenderable = false; ///< True when there is a complete mesh
};

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
    FarTerrainMesh* m_mesh = nullptr;

    const SphericalTerrainData* m_sphericalTerrainData = nullptr; ///< Shared data pointer
    FarTerrainPatch* m_children = nullptr; ///< Pointer to array of 4 children
};

#endif // FarTerrainPatch_h__
