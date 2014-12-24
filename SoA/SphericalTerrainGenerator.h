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

#include "SphericalTerrainPatch.h"
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

class TerrainVertex {
public:
    f32v3 position; //12
    f32v3 tangent; //24
    ColorRGB8 color; //27
    ui8 padding; //28
    ui8v2 texCoords; //30
    ui8 temperature; //31
    ui8 humidity; //32
};

class WaterVertex {
public:
    f32v3 position; //12
    f32v3 tangent; //24
    ColorRGB8 color; //27
    ui8 temperature; //28
    f32v2 texCoords; //36
    float depth; //40
};

class SphericalTerrainGenerator {
public:
    SphericalTerrainGenerator(float radius,
                              SphericalTerrainMeshManager* meshManager,
                              vg::GLProgram* genProgram,
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

    void buildSkirts();

    void addWater(int z, int x);

    void tryAddWaterVertex(int z, int x);

    void tryAddWaterQuad(int z, int x);

    /// TODO: THIS IS REUSABLE
    void generateIndices(VGIndexBuffer& ibo, bool ccw);

    static const int PATCHES_PER_FRAME = 16;

    // PATCH_WIDTH * 4 is for skirts
    static const int VERTS_SIZE = PATCH_SIZE + PATCH_WIDTH * 4;
    static TerrainVertex verts[VERTS_SIZE];
    static WaterVertex waterVerts[VERTS_SIZE];
    static ui16 waterIndexGrid[PATCH_WIDTH][PATCH_WIDTH];
    static ui16 waterIndices[SphericalTerrainPatch::INDICES_PER_PATCH];
    static bool waterQuads[PATCH_WIDTH - 1][PATCH_WIDTH - 1];

    /// Meshing vars
    int m_index;
    int m_waterIndex;
    int m_waterIndexCount;
    float m_vertWidth;
    float m_radius;
    i32v3 m_coordMapping;
    f32v3 m_startPos;
    bool m_ccw;

    int m_patchCounter = 0;
    TerrainGenTextures m_textures[PATCHES_PER_FRAME];
    TerrainGenDelegate* m_delegates[PATCHES_PER_FRAME];
    VGBuffer m_pbos[PATCHES_PER_FRAME];

    VGFramebuffer m_normalFbo = 0;
    ui32v2 m_heightMapDims;

    SphericalTerrainMeshManager* m_meshManager = nullptr;

    vcore::RPCManager m_rpcManager;

    vg::GLProgram* m_genProgram;
    vg::GLProgram* m_normalProgram;

    static VGIndexBuffer m_cwIbo; ///< Reusable CW IBO
    static VGIndexBuffer m_ccwIbo; ///< Reusable CCW IBO

    VGUniform unCornerPos;
    VGUniform unCoordMapping;
    VGUniform unPatchWidth;
    VGUniform unHeightMap;
    VGUniform unWidth;
    VGUniform unTexelWidth;

    vg::FullQuadVBO m_quad;

    static float m_heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][3];
};

#endif // SphericalTerrainGenerator_h__