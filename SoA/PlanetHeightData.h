///
/// PlanetHeightData.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 9 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Heightmap data for planets
///

#pragma once

#ifndef PlanetHeightData_h__
#define PlanetHeightData_h__

struct Biome;

struct PlanetHeightData {
    Biome* biome;
    f32 height; ///< Height in voxels
    ui16 surfaceBlock;
    ui8 temperature;
    ui8 rainfall;
    ui8 depth; // For water, but is this needed?
    ui8 flags; // TODO(Ben): Bitfield
};

#endif // PlanetHeightData_h__
