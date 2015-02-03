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

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GBuffer.h>
#include <Vorb/RPC.h>
#include <Vorb/VorbPreDecl.inl>

#include "VoxelSpaceConversions.h"
#include "SphericalTerrainPatch.h"
#include "TerrainGenTextures.h"
#include "SphericalTerrainPatchMesher.h"

class TerrainGenDelegate;
class RawGenDelegate;
class PlanetGenData;
DECL_VG(class TextureRecycler)

class SphericalTerrainGenerator {
public:
    SphericalTerrainGenerator(SphericalTerrainMeshManager* meshManager,
                              PlanetGenData* planetGenData,
                              vg::GLProgram* normalProgram,
                              vg::TextureRecycler* normalMapRecycler);
    ~SphericalTerrainGenerator();

    // Do this on the openGL thread
    void update();

    void generateTerrain(TerrainGenDelegate* data);

    void generateRawHeightmap(RawGenDelegate* data);

    void invokeRawGen(vcore::RPC* so) {
        // TODO(Ben): Change second param to false
        m_rawRpcManager.invoke(so, false);
    }

    void invokePatchTerrainGen(vcore::RPC* so) {
        m_patchRpcManager.invoke(so, false);
    }
private:

    void updatePatchGeneration();

    void updateRawGeneration();

    static const int PATCHES_PER_FRAME = 8;
    static const int RAW_PER_FRAME = 3;

    int m_dBufferIndex = 0; ///< Index for double buffering

    int m_patchCounter[2];
    TerrainGenTextures m_patchTextures[2][PATCHES_PER_FRAME];
    TerrainGenDelegate* m_patchDelegates[2][PATCHES_PER_FRAME];
    VGBuffer m_patchPbos[2][PATCHES_PER_FRAME];

    int m_rawCounter[2];
    TerrainGenTextures m_rawTextures[2][RAW_PER_FRAME];
    RawGenDelegate* m_rawDelegates[2][RAW_PER_FRAME];
    VGBuffer m_rawPbos[2][RAW_PER_FRAME];

    VGFramebuffer m_normalFbo = 0;
    ui32v2 m_heightMapDims;

    vcore::RPCManager m_patchRpcManager; /// RPC manager for mesh height maps
    vcore::RPCManager m_rawRpcManager; /// RPC manager for raw height data requests

    PlanetGenData* m_planetGenData = nullptr; ///< Planetary data
    vg::GLProgram* m_genProgram = nullptr;
    vg::GLProgram* m_normalProgram = nullptr;

    vg::TextureRecycler* m_normalMapRecycler = nullptr;

    VGUniform unCornerPos;
    VGUniform unCoordMults;
    VGUniform unCoordMapping;
    VGUniform unPatchWidth;
    VGUniform unHeightMap;
    VGUniform unWidth;
    VGUniform unTexelWidth;

    vg::FullQuadVBO m_quad;

    SphericalTerrainPatchMesher mesher;

    static float m_heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][4];
};

#endif // SphericalTerrainGenerator_h__