///
/// PlanetHeightData.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 9 Jun 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Heightmap data for planets
///

#pragma once

#ifndef PlanetHeightData_h__
#define PlanetHeightData_h__

struct Biome;

struct PlanetHeightData {
    const Biome* biome;
    f32 height; ///< Height in voxels
    ui16 flora;
    ui8 temperature;
    ui8 humidity;
    ui8 flags; // TODO(Ben): Bitfield
};

#endif // PlanetHeightData_h__
