///
/// TerrainPatchMeshManager.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Simple container for terrain meshes. Each planet gets one
///
#include "TerrainPatch.h"

#pragma once

#ifndef TerrainPatchMeshManager_h__
#define TerrainPatchMeshManager_h__

#include <RPC.h>
#include <Vorb/VorbPreDecl.inl>

class Camera;
struct PlanetGenData;
class TerrainPatchData;
class TerrainPatchMesh;

DECL_VG(class TextureRecycler;
        class GLProgram)

class TerrainPatchMeshManager {
public:
    TerrainPatchMeshManager(const PlanetGenData* planetGenData,
                                vg::TextureRecycler* normalMapRecycler) :
        m_planetGenData(planetGenData),
        m_normalMapRecycler(normalMapRecycler) {
        // Empty
    }
    ~TerrainPatchMeshManager();
    /// Draws the meshes
    /// @param relativePos: Relative position of the camera
    /// @param Camera: The camera
    /// @param rot: Rotation matrix
    /// @param program: Shader program for rendering terrain
    /// @param waterProgram: Shader program for rendering water
    void drawSphericalMeshes(const f64v3& relativePos, const Camera* camera,
              const f32m4& rot,
              vg::GLProgram* program, vg::GLProgram* waterProgram);
    void drawFarMeshes(const f64v3& relativePos, const Camera* camera,
                       vg::GLProgram* program, vg::GLProgram* waterProgram);

    /// Adds a mesh 
    /// @param mesh: Mesh to add
    void addMesh(TerrainPatchMesh* mesh, bool isSpherical);

private:

    const PlanetGenData* m_planetGenData = nullptr; ///< Planetary data
    vg::TextureRecycler* m_normalMapRecycler = nullptr; ///< Recycler for normal maps
    std::vector<TerrainPatchMesh*> m_meshes; ///< All meshes
    std::vector<TerrainPatchMesh*> m_waterMeshes; ///< Meshes with water active
    std::vector<TerrainPatchMesh*> m_farMeshes; ///< All meshes
    std::vector<TerrainPatchMesh*> m_farWaterMeshes; ///< Meshes with water active
};

#endif // TerrainPatchMeshManager_h__