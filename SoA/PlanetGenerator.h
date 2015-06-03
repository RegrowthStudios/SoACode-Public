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
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLRenderTarget.h>
#include <Vorb/graphics/gtypes.h>
#include <Vorb/graphics/GLProgram.h>

#include <random>
#include <vector>

struct PlanetGenData;

class PlanetGenerator {
public:
    PlanetGenerator();
    void dispose(vcore::RPCManager* glrpc);

    CALLEE_DELETE PlanetGenData* generateRandomPlanet(SpaceObjectType type, vcore::RPCManager* glrpc = nullptr );
private:
    CALLEE_DELETE PlanetGenData* generatePlanet(vcore::RPCManager* glrpc);
    CALLEE_DELETE PlanetGenData* generateAsteroid(vcore::RPCManager* glrpc);
    CALLEE_DELETE PlanetGenData* generateComet(vcore::RPCManager* glrpc);
    VGTexture getRandomColorMap(vcore::RPCManager* glrpc, bool shouldBlur);
    void getRandomTerrainFuncs(OUT std::vector<TerrainFuncKegProperties>& funcs,
                               TerrainStage func,
                               const std::uniform_int_distribution<int>& funcsRange,
                               const std::uniform_int_distribution<int>& octavesRange,
                               const std::uniform_real_distribution<f32>& freqRange,
                               const std::uniform_real_distribution<f32>& heightMinRange,
                               const std::uniform_real_distribution<f32>& heightWidthRange);

    vg::FullQuadVBO m_quad;
    vg::GLRenderTarget m_targets[2];
    vg::GLProgram m_blurPrograms[2];
    std::mt19937 m_generator = std::mt19937(36526);
};

#endif // PlanetGenerator_h__
