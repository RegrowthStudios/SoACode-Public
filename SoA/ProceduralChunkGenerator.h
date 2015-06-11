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
class NChunk;

#include "SphericalTerrainCpuGenerator.h"

class ProceduralChunkGenerator {
public:
    void init(PlanetGenData* genData);
    void generateChunk(NChunk* chunk, PlanetHeightData* heightData) const;
    void generateHeightmap(NChunk* chunk, PlanetHeightData* heightData) const;
private:
    PlanetGenData* m_genData = nullptr;
    SphericalTerrainCpuGenerator m_heightGenerator;
};

#endif // ProceduralChunkGenerator_h__
