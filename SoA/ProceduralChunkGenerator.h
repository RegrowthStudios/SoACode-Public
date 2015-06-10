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

class ProceduralChunkGenerator {
public:
    void init(PlanetGenData* genData);
    void generate(NChunk* chunk, PlanetHeightData* heightData) const;
private:
    PlanetGenData* m_genData = nullptr;
};

#endif // ProceduralChunkGenerator_h__
