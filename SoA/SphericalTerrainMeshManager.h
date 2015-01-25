///
/// SphericalTerrainMeshManager.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Simple container for terrain meshes. Each planet gets one
///

#pragma once

#ifndef SphericalTerrainMeshManager_h__
#define SphericalTerrainMeshManager_h__

#include <RPC.h>
#include <Vorb/VorbPreDecl.inl>

class Camera;
class PlanetGenData;
class SphericalTerrainData;
class SphericalTerrainMesh;

DECL_VG(class TextureRecycler;
        class GLProgram)

class SphericalTerrainMeshManager {
public:
    SphericalTerrainMeshManager(const PlanetGenData* planetGenData,
                                vg::TextureRecycler* normalMapRecycler) :
        m_planetGenData(planetGenData),
        m_normalMapRecycler(normalMapRecycler) {
        // Empty
    }
    /// Draws the meshes
    /// @param relativePos: Relative position of the camera
    /// @param Camera: The camera
    /// @param rot: Rotation matrix
    /// @param program: Shader program for rendering terrain
    /// @param waterProgram: Shader program for rendering water
    void draw(const f64v3& relativePos, const Camera* camera,
              const f32m4& rot,
              vg::GLProgram* program, vg::GLProgram* waterProgram);

    /// Adds a mesh 
    /// @param mesh: Mesh to add
    void addMesh(SphericalTerrainMesh* mesh);

private:
    const PlanetGenData* m_planetGenData = nullptr; ///< Planetary data
    vg::TextureRecycler* m_normalMapRecycler = nullptr; ///< Recycler for normal maps
    std::vector<SphericalTerrainMesh*> m_meshes; ///< All meshes
    std::vector<SphericalTerrainMesh*> m_waterMeshes; ///< Meshes with water active
};

#endif // SphericalTerrainMeshManager_h__