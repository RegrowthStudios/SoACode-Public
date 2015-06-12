///
/// SphericalTerrainCpuGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Generates spherical terrain data and meshes for a planet using the CPU.
/// Each planet should own one.
///

#pragma once

#ifndef SphericalTerrainCpuGenerator_h__
#define SphericalTerrainCpuGenerator_h__

#include "TerrainPatch.h"
#include "TerrainPatchMesher.h"
#include "VoxelCoordinateSpaces.h"

struct NoiseBase;

class SphericalTerrainCpuGenerator {
public:
    SphericalTerrainCpuGenerator(TerrainPatchMeshManager* meshManager,
                                 PlanetGenData* planetGenData);
    ~SphericalTerrainCpuGenerator();

    /// Generates a terrain patch: NOTE: This is only here for testing purposes. GPUgen is vastly superior
    /// @param mesh: The mesh handle
    /// @param startPos: Starting position
    /// @param cubeFace: The world cube face
    /// @param width: Width of the patch
    void generateTerrainPatch(OUT TerrainPatchMesh* mesh, const f32v3& startPos, WorldCubeFace cubeFace, float width);

    /// Gets the height at a specific face position.
    /// @param facePosition: The position to query
    /// @return height in meters.
    float getTerrainHeight(const VoxelPosition2D& facePosition);

private:
    /// Gets noise value using terrainFuncs
    /// @param pos: Position to sample noise from
    /// @Param funcs: The terrain functions to use
    /// @return the noise value
    float getNoiseValue(const f32v3& pos, const NoiseBase& funcs);

    TerrainPatchMesher m_mesher; ///< Creates patch meshes
    const PlanetGenData* m_genData = nullptr; ///< Planet generation data
};

#endif // SphericalTerrainCpuGenerator_h__
