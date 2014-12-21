///
/// SphericalTerrainGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Generates spherical terrain and meshes for a planet. 
/// Each planet should own one.
///

#pragma once

#ifndef SphericalTerrainGenerator_h__
#define SphericalTerrainGenerator_h__

class TerrainGenDelegate;

#include "TerrainGenTextures.h"
#include <GLProgram.h>
#include <FullQuadVBO.h>
#include <GBuffer.h>

// Coordinate mapping for rotating 2d grid to quadcube positions
// Pain of i32v3, first is coordinates
const i32v3 CubeCoordinateMappings[6] = {
    i32v3(0, 1, 2), //TOP
    i32v3(1, 0, 2), //LEFT
    i32v3(1, 0, 2), //RIGHT
    i32v3(0, 2, 1), //FRONT
    i32v3(0, 2, 1), //BACK
    i32v3(0, 1, 2) //BOTTOM
};

// Vertex Winding
// True when CCW
const bool CubeWindings[6] = {
    true,
    true,
    false,
    false,
    true,
    false
};

// Multipliers for coordinate mappings
const f32v3 CubeCoordinateMults[6] = {
    f32v3(1.0f, 1.0f, 1.0f), //TOP
    f32v3(1.0f, -1.0f, 1.0f), //LEFT
    f32v3(1.0f, 1.0f, 1.0f), //RIGHT
    f32v3(1.0f, 1.0f, 1.0f), //FRONT
    f32v3(1.0f, -1.0f, 1.0f), //BACK
    f32v3(1.0f, -1.0f, 1.0f) //BOTTOM
};

class SphericalTerrainGenerator {
public:
    SphericalTerrainGenerator(float radius, vg::GLProgram* genProgram,
                              vg::GLProgram* normalProgram);
    ~SphericalTerrainGenerator();

    // Do this on the openGL thread
    void update();

    void generateTerrain(TerrainGenDelegate* data);

    void invokeTerrainGen(vcore::RPC* so) {
        m_rpcManager.invoke(so, false);
    }
private:
    /// Generates mesh using heightmap
    void buildMesh(TerrainGenDelegate* data);

    /// TODO: THIS IS REUSABLE
    void generateIndices(TerrainGenDelegate* data);

    float m_radius;

    TerrainGenTextures m_textures;

    vcore::RPCManager m_rpcManager;

    vg::GLProgram* m_genProgram;
    vg::GLProgram* m_normalProgram;

    VGUniform unCornerPos;
    VGUniform unCoordMapping ;
    VGUniform unPatchWidth;

    vg::FullQuadVBO m_quad;

    vg::GBuffer gbuffer;
};

#endif // SphericalTerrainGenerator_h__