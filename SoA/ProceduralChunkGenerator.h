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

#include "SphericalTerrainCpuGenerator.h"

class ProceduralChunkGenerator {
public:
    void init(PlanetGenData* genData);
    void generateChunk(Chunk* chunk, PlanetHeightData* heightData) const;
    void generateHeightmap(Chunk* chunk, PlanetHeightData* heightData) const;
private:
    const BlockLayer& getBlockLayer(int depth) const;

    PlanetGenData* m_genData = nullptr;
    SphericalTerrainCpuGenerator m_heightGenerator;
};

#endif // ProceduralChunkGenerator_h__
