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

struct PlanetHeightData {
    ui32 height;
    ui16 surfaceBlock;
    ui8 temperature;
    ui8 rainfall;
    ui8 depth;
    ui8 flags;  
};

#endif // PlanetHeightData_h__
