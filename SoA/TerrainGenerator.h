#pragma once
#include <Vorb/Vorb.h>

#include "Biome.h"

const int FREEZETEMP = 50;

class HeightData {
public:
    int height;
    int temperature;
    int rainfall;
    int snowDepth; // TODO(Ben): THESE AREN'T NEEDED
    int sandDepth;
    int flags;
    GLubyte depth;
    Biome *biome = nullptr;
    GLushort surfaceBlock;
};

// TODO(Ben): This is absolutely awful
class TerrainGenerator
{
public:
 
};

const int NumTerrainFunctions = 11;
const nString TerrainFunctionNames[NumTerrainFunctions] = { "Default", "Small Mts.", "Large Mts.+Lakes", "Hills+Plateaus", "Seaside Cliffs",
    "Ridged", "Billowy Hills", "Jagged Cliffs", "Huge Mts.", "Volcano", "Canyons" };

extern nString TerrainFunctionHelps[NumTerrainFunctions];
