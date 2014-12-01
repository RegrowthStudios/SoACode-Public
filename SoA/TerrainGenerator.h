#pragma once
#include "global.h"

#include "Biome.h"
#include "Vorb.h"
#include "IVoxelMapper.h"

// TODO: Remove This
using namespace std;

const int FREEZETEMP = 50;

class HeightData {
public:
    GLint height;
    GLint temperature;
    GLint rainfall;
    GLint snowDepth;
    GLint sandDepth;
    GLint flags;
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
const string TerrainFunctionNames[NumTerrainFunctions] = { "Default", "Small Mts.", "Large Mts.+Lakes", "Hills+Plateaus", "Seaside Cliffs",
    "Ridged", "Billowy Hills", "Jagged Cliffs", "Huge Mts.", "Volcano", "Canyons" };

extern string TerrainFunctionHelps[NumTerrainFunctions];