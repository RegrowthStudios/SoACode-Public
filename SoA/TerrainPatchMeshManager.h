///
/// TerrainPatchMeshManager.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Dec 2014
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Simple container for terrain meshes. Each planet gets one
///
#include "TerrainPatch.h"

#pragma once

#ifndef TerrainPatchMeshManager_h__
#define TerrainPatchMeshManager_h__

#include <Vorb/RPC.h>
#include <Vorb/VorbPreDecl.inl>

class Camera;
class TerrainPatchMesh;
struct AtmosphereComponent;
struct PlanetGenData;
struct TerrainPatchData;

DECL_VG(class TextureRecycler;
        class GLProgram)

class TerrainPatchMeshManager {
public:
    TerrainPatchMeshManager(const PlanetGenData* planetGenData) :
        m_planetGenData(planetGenData){
        // Empty
    }
    ~TerrainPatchMeshManager();

    void update();

    /// Draws the spherical meshes
    /// @param relativePos: Relative position of the camera
    /// @param Camera: The camera
    /// @param orientation: Orientation quaternion
    /// @param program: Shader program for rendering terrain
    /// @param waterProgram: Shader program for rendering water
    /// @param lightDir: Normalized direction to light source
    /// @param aCmp: Atmosphere component for rendering
    /// @param drawSkirts: True when you want to also draw skirts
    void drawSphericalMeshes(const f64v3& relativePos,
                             const Camera* camera,
                             const f64q& orientation,
                             vg::GLProgram& program, vg::GLProgram& waterProgram,
                             const f32v3& lightDir,
                             f32 alpha,
                             const f32 zCoef,
                             const AtmosphereComponent* aCmp,
                             bool drawSkirts);
    /// Draws the far meshes
    /// @param relativePos: Relative position of the camera
    /// @param Camera: The camera state
    /// @param orientation: Orientation quaternion
    /// @param program: Shader program for rendering terrain
    /// @param waterProgram: Shader program for rendering water
    /// @param lightDir: Normalized direction to light source
    /// @param radius: Radius of the planet in KM
    /// @param aCmp: Atmosphere component for rendering
    /// @param drawSkirts: True when you want to also draw skirts
    void drawFarMeshes(const f64v3& relativePos,
                       const Camera* camera,
                       vg::GLProgram& program, vg::GLProgram& waterProgram,
                       const f32v3& lightDir,
                       f32 alpha, f32 radius,
                       const f32 zCoef,
                       const AtmosphereComponent* aCmp,
                       bool drawSkirts);

    /// Adds a mesh 
    void addMesh(TerrainPatchMesh* mesh);
    /// Adds a mesh from a worker thread
    void addMeshAsync(TerrainPatchMesh* mesh);

    /// Updates distances and Sorts meshes
    void sortSpericalMeshes(const f64v3& relPos);

    /// Updates distances and Sorts meshes
    void sortFarMeshes(const f64v3& relPos);

private:
    void setScatterUniforms(vg::GLProgram& program, const f64v3& relPos, const AtmosphereComponent* aCmp);

    moodycamel::ConcurrentQueue<TerrainPatchMesh*> m_meshesToAdd;

    const PlanetGenData* m_planetGenData = nullptr; ///< Planetary data
    std::vector<TerrainPatchMesh*> m_meshes; ///< All meshes
    std::vector<TerrainPatchMesh*> m_waterMeshes; ///< Meshes with water active
    std::vector<TerrainPatchMesh*> m_farMeshes; ///< All meshes
    std::vector<TerrainPatchMesh*> m_farWaterMeshes; ///< Meshes with water active
};

#endif // TerrainPatchMeshManager_h__
