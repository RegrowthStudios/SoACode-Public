#include "stdafx.h"
#include "WorldStructs.h"

#include "BlockData.h"
#include "Chunk.h"
#include "Options.h"
#include "GameManager.h"
#include "GLProgramManager.h"
#include "RenderTask.h"
#include "Texture2d.h"

MultiplePreciseTimer globalMultiplePreciseTimer; ///< for easy global benchmarking

class Item *ObjectList[OBJECT_LIST_SIZE];

Marker::Marker(const glm::dvec3 &Pos, nString Name, glm::vec3 Color) : pos(Pos), name(Name), dist(0.0)
{
    // TODO(Ben): implement
}

void Marker::Draw(glm::mat4 &VP, const glm::dvec3 &playerPos)
{
    // TODO(Ben): implement
}

Biome::Biome()
{
    memset(this, 0, sizeof(Biome)); //zero the memory
    maxHeight = 999999;
    maxHeightSlopeLength = 0;
    looseSoilDepth = 4;
    name = "NO BIOME";
    vecIndex = -1;
    surfaceBlock = DIRTGRASS;
    underwaterBlock = SAND;
    beachBlock = SAND;
    filename = "";

    //default layers
    int i = 0;
    for (; i < 3; i++) surfaceLayers[i] = DIRT;
    for (; i < 10; i++) surfaceLayers[i] = GRAVEL;
    for (; i < 24; i++) surfaceLayers[i] = SLATE;
    for (; i < 35; i++) surfaceLayers[i] = LIMESTONE;
    for (; i < 55; i++) surfaceLayers[i] = SLATE;
    for (; i < 80; i++) surfaceLayers[i] = GNEISS;
    for (; i < 120; i++) surfaceLayers[i] = BASALT;
    for (; i < 160; i++) surfaceLayers[i] = GRANITE;
    for (; i < SURFACE_DEPTH; i++) surfaceLayers[i] = STONE;
}