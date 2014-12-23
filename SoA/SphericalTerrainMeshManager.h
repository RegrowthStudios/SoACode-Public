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
#include <GLProgram.h>

class SphericalTerrainMesh;
class PlanetGenData;

class SphericalTerrainMeshManager {
public:
    SphericalTerrainMeshManager(PlanetGenData* planetGenData) :
        m_planetGenData(planetGenData) {
        // Empty
    }
    /// Draws the meshes
    /// @param cameraPos: Position of the camera
    /// @param VP: View-Projection matrix
    /// @param program: Shader program for rendering terrain
    /// @param waterProgram: Shader program for rendering water
    void draw(const f64v3& cameraPos, const f32m4& V, const f32m4& VP,
              vg::GLProgram* program, vg::GLProgram* waterProgram);

    void addMesh(SphericalTerrainMesh* mesh);

private:
    PlanetGenData* m_planetGenData = nullptr;
    std::vector<SphericalTerrainMesh*> m_meshes; ///< All meshes
    std::vector<SphericalTerrainMesh*> m_waterMeshes; ///< Meshes with water active
};

#endif // SphericalTerrainMeshManager_h__