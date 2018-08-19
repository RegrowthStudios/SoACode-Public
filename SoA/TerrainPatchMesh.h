///
/// TerrainPatchMesh.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Feb 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Defines mesh for TerrainPatches
///

#pragma once

#ifndef TerrainPatchMesh_h__
#define TerrainPatchMesh_h__

#include <Vorb/graphics/gtypes.h>
#include <Vorb/VorbPreDecl.inl>

#include "VoxelCoordinateSpaces.h"
#include "TerrainPatchConstants.h"

class Camera;
DECL_VG(class TextureRecycler);
DECL_VG(class GLProgram);

/// Vertex for terrain patch
class TerrainVertex {
public:
    f32v3 position; //12
    f32v3 normal; //24
    ColorRGB8 color; //27
    ui8 padding; //28
    ui8 temperature; //29
    ui8 humidity; //30
    ui8 padding2[2]; //32
};
/// Water vertex for terrain patch
class WaterVertex {
public:
    f32v3 position; //12
    f32v3 tangent; //24
    ColorRGB8 color; //27
    ui8 temperature; //28
    float depth; //32
};

class TerrainPatchMesh {
public:
    friend class FarTerrainPatch;
    friend class TerrainPatch;
    friend class TerrainPatchMeshManager;
    friend class TerrainPatchMeshTask;
    friend class TerrainPatchMesher;
    TerrainPatchMesh(WorldCubeFace cubeFace, bool isSpherical) :
        m_cubeFace(cubeFace), m_isSpherical(isSpherical) {}
    ~TerrainPatchMesh();

    /// Recycles the normal map
    /// @param recycler: Recycles the texture
    void recycleNormalMap(vg::TextureRecycler* recycler);

    /// Draws the terrain mesh
    void draw(const f32m4& WVP, const vg::GLProgram& program,
              bool drawSkirts) const;

    /// Draws the water mesh
    void drawWater(const f32m4& WVP, const vg::GLProgram& program) const;

    /// Draws the terrain mesh as a far terrain mesh
    void drawAsFarTerrain(const f64v3& relativePos, const f32m4& VP,
                          const vg::GLProgram& program,
                          bool drawSkirts) const;

    /// Draws the water mesh as a far terrain mesh
    void drawWaterAsFarTerrain(const f64v3& relativePos, const f32m4& VP,
                               const vg::GLProgram& program) const;


    /// Gets the point closest to the observer
    /// @param camPos: Position of observer
    /// @return the closest point on the aabb
    f32v3 getClosestPoint(const f32v3& camPos) const;
    f64v3 getClosestPoint(const f64v3& camPos) const;
    const bool& getIsSpherical() const { return m_isSpherical; }

    f64 distance2 = 100000000000.0;
private:
    VGVertexArray m_vao = 0; ///< Vertex array object
    VGVertexBuffer m_vbo = 0; ///< Vertex buffer object
  
    VGVertexArray m_wvao = 0; ///< Water vertex array object
    VGVertexBuffer m_wvbo = 0; ///< Water Vertex buffer object
    VGIndexBuffer m_wibo = 0; ///< Water Index Buffer Object

    f32v3 m_aabbPos = f32v3(0.0f); ///< Bounding box origin
    f32v3 m_aabbDims = f32v3(0.0f); ///< Bounding box dims
    f32v3 m_aabbCenter = f32v3(0.0f); ///< Center of the bounding box
    f32 m_boundingSphereRadius = 0.0f; ///< Radius of sphere for frustum checks
    WorldCubeFace m_cubeFace;

    std::vector<ui8> m_meshDataBuffer; ///< Stores mesh data for terrain and water in bytes

    int m_waterIndexCount = 0;
    int m_waterVertexCount = 0;

    volatile bool m_shouldDelete = false; ///< True when the mesh should be deleted
    bool m_isRenderable = false; ///< True when there is a complete mesh
    bool m_isSpherical = false;
};

#endif // TerrainPatchMesh_h__
