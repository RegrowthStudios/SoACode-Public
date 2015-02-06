///
/// NoiseShaderGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 5 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Generates shaders for procedural terrain generation
///

#pragma once

#ifndef NoiseShaderGenerator_h__
#define NoiseShaderGenerator_h__

#include <Vorb/graphics/Texture.h>
#include <Vorb/VorbPreDecl.inl>

#include "Biome.h"

DECL_VG(class GLProgram)
struct PlanetGenData;
struct TerrainFuncs;

class NoiseShaderGenerator {
public:
    CALLER_DELETE vg::GLProgram* generateProgram(PlanetGenData* genData,
                                   TerrainFuncs& baseTerrainFuncs,
                                   TerrainFuncs& tempTerrainFuncs,
                                   TerrainFuncs& humTerrainFuncs);

    CALLER_DELETE vg::GLProgram* getDefaultProgram();
private:
    void addNoiseFunctions(nString& fSource, const nString& variable, const TerrainFuncs& funcs);

    void addBiomes(nString& fSource, PlanetGenData* genData);

    void dumpShaderCode(std::ostream& stream, nString source, bool addLineNumbers);
};

#endif // NoiseShaderGenerator_h__
