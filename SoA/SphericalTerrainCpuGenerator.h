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
#include "PlanetData.h"

struct NoiseBase;
struct PlanetHeightData;

class SphericalTerrainCpuGenerator {
public:
    void init(const PlanetGenData* planetGenData);

    /// Gets the height at a specific face position.
    void generateHeight(OUT PlanetHeightData& height, const VoxelPosition2D& facePosition) const;

    f64 getHeight(const VoxelPosition2D& facePosition) const;

    f64 getHeightValue(const f64v3& pos) const;
    f64 getTemperatureValue(const f64v3& pos, const f64v3& normal, f64 height) const;
    f64 getHumidityValue(const f64v3& pos, const f64v3& normal, f64 height) const;

    /// Calculates temperature based on angle with equator
    /// @param range: The range to scale between
    /// @angle: Angle from equator
    /// @param baseTemp: Base temperature at equator
    static f64 calculateTemperature(f64 range, f64 angle, f64 baseTemp);
    /// Calculates humidity based on angle with equator
    /// @param range: The range to scale between
    /// @angle: Angle from equator
    /// @param baseTemp: Base humidity at equator
    static f64 calculateHumidity(f64 range, f64 angle, f64 baseHum);

    // Computes angle from normalized position
    static f64 computeAngleFromNormal(const f64v3& normal);

    const PlanetGenData* getGenData() const { return m_genData; }
private:
    /// Gets noise value using terrainFuncs
    /// @return the noise value
    f64 getNoiseValue(const f64v3& pos,
                      const Array<TerrainFuncKegProperties>& funcs,
                      f64* modifier,
                      const TerrainOp& op) const;

    const PlanetGenData* m_genData = nullptr; ///< Planet generation data
};

#endif // SphericalTerrainCpuGenerator_h__
