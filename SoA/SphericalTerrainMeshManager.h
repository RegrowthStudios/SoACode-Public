///
/// SphericalTerrainMeshManager.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Simple container for terrain meshes
///

#pragma once

#ifndef SphericalTerrainMeshManager_h__
#define SphericalTerrainMeshManager_h__

#include <RPC.h>
#include <GLProgram.h>

class SphericalTerrainMesh;

class SphericalTerrainMeshManager {
public:

    /// Draws the meshes
    /// @param cameraPos: Position of the camera
    /// @param VP: View-Projection matrix
    /// @param program: Shader program for rendering
    void draw(const f64v3& cameraPos, const f32m4& V, const f32m4& VP, vg::GLProgram* program);

    void addMesh(SphericalTerrainMesh* mesh);

private:
    std::vector<SphericalTerrainMesh*> m_meshes;
};

#endif // SphericalTerrainMeshManager_h__