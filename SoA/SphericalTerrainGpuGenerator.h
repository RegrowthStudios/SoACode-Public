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
struct PlanetGenData;
DECL_VG(class TextureRecycler)

class SphericalTerrainGpuGenerator {
public:
    SphericalTerrainGpuGenerator(SphericalTerrainMeshManager* meshManager,
                              PlanetGenData* planetGenData,
                              vg::GLProgram* normalProgram,
                              vg::TextureRecycler* normalMapRecycler);
    ~SphericalTerrainGpuGenerator();

    /// Updates the generator. Call from OpenGL thread
    void update();

    /// Generates a terrain patch for spherical terrain
    /// @param data: The delegate data
    void generateTerrainPatch(TerrainGenDelegate* data);

    /// Generates a raw heightmap
    /// @param data: The delegate data
    void generateRawHeightmap(RawGenDelegate* data);

    /// Invokes a raw heighmap generation with RPC
    /// @so: The RPC
    void invokeRawGen(vcore::RPC* so) {
        // TODO(Ben): Change second param to false
        m_rawRpcManager.invoke(so, false);
    }
    /// Invokes a patch generation with RPC
    /// @so: The RPC
    void invokePatchTerrainGen(vcore::RPC* so) {
        m_patchRpcManager.invoke(so, false);
    }

    const PlanetGenData* getPlanetGenData() { return m_planetGenData; }
private:
    /// Updates terrain patch generation
    void updatePatchGeneration();
    /// Updates raw heightmap generation
    void updateRawGeneration();

    static const int PATCHES_PER_FRAME = 6; ///< Number of terrain patches to process per frame
    static const int RAW_PER_FRAME = 3; ///< Number of raw heightmaps to process per frame

    int m_dBufferIndex = 0; ///< Index for double buffering

    int m_patchCounter[2]; ///< Double buffered patch counter
    TerrainGenTextures m_patchTextures[2][PATCHES_PER_FRAME]; ///< Double buffered textures for patch gen
    TerrainGenDelegate* m_patchDelegates[2][PATCHES_PER_FRAME]; ///< Double buffered delegates for patch gen
    VGBuffer m_patchPbos[2][PATCHES_PER_FRAME]; ///< Double bufferd PBOs for patch gen

    int m_rawCounter[2]; ///< Double buffered raw counter
    TerrainGenTextures m_rawTextures[2][RAW_PER_FRAME]; ///< Double buffered textures for raw gen
    RawGenDelegate* m_rawDelegates[2][RAW_PER_FRAME]; ///< Double buffered delegates for raw gen
    VGBuffer m_rawPbos[2][RAW_PER_FRAME]; ///< Double bufferd PBOs for raw gen

    VGFramebuffer m_normalFbo = 0; ///< FBO for normal map generation
    ui32v2 m_heightMapDims; ///< Dimensions of heightmap

    vcore::RPCManager m_patchRpcManager; /// RPC manager for mesh height maps
    vcore::RPCManager m_rawRpcManager; /// RPC manager for raw height data requests

    PlanetGenData* m_planetGenData = nullptr; ///< Planetary data
    vg::GLProgram* m_genProgram = nullptr; ///< Generation program
    vg::GLProgram* m_normalProgram = nullptr; ///< Normal map gen program

    vg::TextureRecycler* m_normalMapRecycler = nullptr; ///< Recycles the normal maps

    /// Uniforms
    VGUniform unCornerPos;
    VGUniform unCoordMults;
    VGUniform unCoordMapping;
    VGUniform unPatchWidth;
    VGUniform unHeightMap;
    VGUniform unWidth;
    VGUniform unTexelWidth;

    vg::FullQuadVBO m_quad; ///< Quad for rendering

    SphericalTerrainPatchMesher mesher; ///< Creates patch meshes

    static float m_heightData[PATCH_HEIGHTMAP_WIDTH][PATCH_HEIGHTMAP_WIDTH][4]; ///< Stores height data
};

#endif // SphericalTerrainGenerator_h__