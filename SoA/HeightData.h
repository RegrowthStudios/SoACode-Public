///
/// HeightData.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 16 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Heightmap data for generation
///

#pragma once

#ifndef HeightData_h__
#define HeightData_h__

class Biome;

class HeightData {
public:
    int height;
    int temperature; // TODO(Ben): ui16 or ui8?
    int rainfall;
    int snowDepth; // TODO(Ben): THESE AREN'T NEEDED
    int sandDepth;
    int flags; // UI8 bitfield?
    ui8 depth;
    Biome *biome = nullptr;
    ui16 surfaceBlock; // TODO(Ben): Get rid of this
};

#endif // HeightData_h__
