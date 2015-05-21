///
/// PlanetGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 16 Apr 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Generates random planet data
///

#pragma once

#ifndef PlanetGenerator_h__
#define PlanetGenerator_h__

#include "PlanetData.h"
#include "SpaceSystemLoadStructs.h"
#include <Vorb/RPC.h>
#include <Vorb/graphics/gtypes.h>
#include <vector>
#include <random>

struct PlanetGenData;

class PlanetGenerator {
public:
    static CALLEE_DELETE PlanetGenData* generateRandomPlanet(SpaceObjectType type, vcore::RPCManager* glrpc = nullptr );
private:
    static CALLEE_DELETE PlanetGenData* generatePlanet(vcore::RPCManager* glrpc);
    static CALLEE_DELETE PlanetGenData* generateAsteroid(vcore::RPCManager* glrpc);
    static CALLEE_DELETE PlanetGenData* generateComet(vcore::RPCManager* glrpc);
    static VGTexture getRandomColorMap(vcore::RPCManager* glrpc);
    static void getRandomTerrainFuncs(OUT std::vector<TerrainFuncKegProperties>& funcs,
                                      const std::uniform_int_distribution<int>& funcsRange,
                                      const std::uniform_int_distribution<int>& octavesRange,
                                      const std::uniform_real_distribution<f32>& freqRange,
                                      const std::uniform_real_distribution<f32>& heightMinRange,
                                      const std::uniform_real_distribution<f32>& heightWidthRange);

    static std::mt19937 generator;
};

#endif // PlanetGenerator_h__
