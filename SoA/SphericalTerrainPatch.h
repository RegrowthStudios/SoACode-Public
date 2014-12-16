///
/// SphericalTerrainPatch.h
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

#ifndef SphericalTerrainPatch_h__
#define SphericalTerrainPatch_h__

#include "TerrainGenerator.h"

#include "gtypes.h"
#include "GLProgram.h"

enum class CubeFace { TOP, LEFT, RIGHT, FRONT, BACK, BOTTOM };

class Camera;

#define PATCH_WIDTH 5

// Shared terrain data for spherical planet terrain
class SphericalTerrainData {
public:
    friend class SphericalTerrainComponent;

    SphericalTerrainData(f64 radius,
                         f64 patchWidth) :
        m_radius(radius),
        m_patchWidth(patchWidth) {
        // Empty
    }

    const f64& getRadius() const { return m_radius; }
private:
    f64 m_radius; ///< Radius of the planet in KM
    f64 m_patchWidth; ///< Width of a patch in KM
};

class SphericalTerrainPatch {
public:
    SphericalTerrainPatch() { };
    ~SphericalTerrainPatch();
    
    /// Initializes the patch
    /// @param gridPosition: Position on the 2d face grid
    /// @param sphericalTerrainData: Shared data
    /// @param width: Width of the patch in KM
    void init(const f64v2& gridPosition,
              CubeFace cubeFace,
              const SphericalTerrainData* sphericalTerrainData,
              f64 width);

    /// Updates the patch
    /// @param cameraPos: Position of the camera
    void update(const f64v3& cameraPos);

    /// Frees resources
    void destroy();

    /// Temporary? Draws the patch
    /// @param cameraPos: Position of the camera
    /// @param VP: View-Projection matrix
    /// @param program: Shader program for rendering
    void draw(const f64v3& cameraPos, const f32m4& VP, vg::GLProgram* program);

    /// @return true if it has a generated mesh
    bool hasMesh() const { return (m_vbo != 0); }

    /// @return true if it has a mesh, or all of its children are
    /// renderable.
    bool isRenderable() const;

private:
    /// Frees mesh resources
    void destroyMesh();

    /// Generates mesh from a heightmap
    void generateMesh(float heightData[PATCH_WIDTH][PATCH_WIDTH]);

    f64v2 m_gridPosition = f64v2(0.0); ///< Position on 2D grid
    f64v3 m_worldPosition = f64v3(0.0); ///< Position relative to world
    f32v3 m_boundingBox; ///< Bounding box that contains all points
    f64 m_distance = 1000000000.0; ///< Distance from camera
    int m_lod = 0; ///< Level of detail
    CubeFace m_cubeFace; ///< Which cube face grid it is on

    f64 m_width = 0.0; ///< Width of the patch in KM

    VGVertexArray m_vao = 0; ///< Vertex array object
    VGVertexBuffer m_vbo = 0; ///< Vertex buffer object
    VGIndexBuffer m_ibo = 0; ///< Index buffer object

    const SphericalTerrainData* m_sphericalTerrainData = nullptr; ///< Shared data pointer
    SphericalTerrainPatch* m_children = nullptr; ///< Pointer to array of 4 children
};

#endif // SphericalTerrainPatch_h__