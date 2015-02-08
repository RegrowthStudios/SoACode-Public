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
#include "VoxelCoordinateSpaces.h"

#include <Vorb/graphics/gtypes.h>
#include <Vorb/graphics/GLProgram.h>

class Camera;
class MeshManager;
class TerrainRpcDispatcher;
class TerrainGenDelegate;
namespace vorb {
    namespace core {
        namespace graphics {
            class TextureRecycler;
        }
    }
}

const int PIXELS_PER_PATCH_NM = 4;
const int PATCH_WIDTH = 33;
const int PATCH_SIZE = PATCH_WIDTH * PATCH_WIDTH;
const int PATCH_NORMALMAP_WIDTH = (PATCH_WIDTH - 1) * PIXELS_PER_PATCH_NM + 2; // + 2 for padding
const int PATCH_HEIGHTMAP_WIDTH = PATCH_NORMALMAP_WIDTH + 2; // + 2 for padding

const int MAX_LOD = 25; ///< Absolute maximum

// Shared terrain data for spherical planet terrain
class SphericalTerrainData {
public:
    friend struct SphericalTerrainComponent;

    SphericalTerrainData(f64 radius,
                         f64 patchWidth) :
        m_radius(radius),
        m_patchWidth(patchWidth) {
        // Empty
    }

    const f64& getRadius() const { return m_radius; }
    const f64& getPatchWidth() const { return m_patchWidth; }
private:
    f64 m_radius; ///< Radius of the planet in KM
    f64 m_patchWidth; ///< Width of a patch in KM
};

class SphericalTerrainMesh {
public:
    friend class SphericalTerrainGpuGenerator;
    friend class SphericalTerrainMeshManager;
    friend class SphericalTerrainPatch;
    friend class SphericalTerrainPatchMesher;
    SphericalTerrainMesh(WorldCubeFace cubeFace) : m_cubeFace(cubeFace) {}
    ~SphericalTerrainMesh();

    /// Recycles the normal map
    /// @param recycler: Recycles the texture
    void recycleNormalMap(vg::TextureRecycler* recycler);

    /// Draws the terrain mesh
    /// @param cameraPos: Relative position of the camera
    /// @param camera: The camera
    /// @param rot: Rotation matrix
    /// @param program: Shader program for rendering
    void draw(const f64v3& relativePos, const Camera* camera,
              const f32m4& rot, vg::GLProgram* program) const;
   
    /// Draws the water mesh
    /// @param relativePos: Relative position of the camera
    /// @param camera: The camera
    /// @param rot: Rotation matrix
    /// @param program: Shader program for rendering
    void drawWater(const f64v3& relativePos, const Camera* camera,
                   const f32m4& rot, vg::GLProgram* program) const;

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

// TODO(Ben): Sorting, Horizon Culling, Atmosphere, Frustum Culling, Bugfixes,
// fix redundant quality changes
class SphericalTerrainPatch {
public:
    SphericalTerrainPatch() { };
    ~SphericalTerrainPatch();
    
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
    SphericalTerrainPatch* m_children = nullptr; ///< Pointer to array of 4 children
};

#endif // SphericalTerrainPatch_h__