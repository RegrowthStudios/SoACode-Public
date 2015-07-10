///
/// ProceduralChunkGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef ProceduralChunkGenerator_h__
#define ProceduralChunkGenerator_h__

struct PlanetGenData;
struct PlanetHeightData;
struct BlockLayer;
class Chunk;

#include "SphericalHeightmapGenerator.h"

class ProceduralChunkGenerator {
public:
    void init(PlanetGenData* genData);
    void generateChunk(Chunk* chunk, PlanetHeightData* heightData) const;
    void generateHeightmap(Chunk* chunk, PlanetHeightData* heightData) const;
private:
    ui32 getBlockLayerIndex(ui32 depth) const;
    ui16 getBlockID(int depth, int mapHeight, int height, BlockLayer& layer) const;

    PlanetGenData* m_genData = nullptr;
    SphericalHeightmapGenerator m_heightGenerator;
};

#endif // ProceduralChunkGenerator_h__
