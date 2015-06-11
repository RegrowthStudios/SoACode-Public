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
struct PlanetHeightData;

class SphericalTerrainCpuGenerator {
public:
    void init(const PlanetGenData* planetGenData);

    /// Generates a terrain patch: NOTE: This is only here for testing purposes. GPUgen is vastly superior
    void generateTerrainPatch(OUT TerrainPatchMesh* mesh, const f32v3& startPos, WorldCubeFace cubeFace, float width) const;

    /// Gets the height at a specific face position.
    void generateHeight(OUT PlanetHeightData& height, const VoxelPosition2D& facePosition) const;

    f64 getHeight(const VoxelPosition2D& facePosition) const;
private:
    /// Gets noise value using terrainFuncs
    /// @param pos: Position to sample noise from
    /// @Param funcs: The terrain functions to use
    /// @return the noise value
    f64 getNoiseValue(const f64v3& pos, const NoiseBase& funcs) const;

  //  TerrainPatchMesher m_mesher; ///< Creates patch meshes
    const PlanetGenData* m_genData = nullptr; ///< Planet generation data
};

#endif // SphericalTerrainCpuGenerator_h__
