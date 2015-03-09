///
/// PlanetData.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 5 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Structs for planetary data
///

#pragma once

#ifndef PlanetData_h__
#define PlanetData_h__

#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/Texture.h>
#include <Vorb/io/Keg.h>

#include "Biome.h"

DECL_VG(class GLProgram);

struct LiquidColorKegProperties {
    nString colorPath = "";
    nString texturePath = "";
    ColorRGB8 tint = ColorRGB8(255, 255, 255);
    f32 depthScale = 1000.0f;
    f32 freezeTemp = -1.0f;
};
KEG_TYPE_DECL(LiquidColorKegProperties);


struct TerrainColorKegProperties {
    nString colorPath = "";
    nString texturePath = "";
    ColorRGB8 tint = ColorRGB8(255, 255, 255);
};
KEG_TYPE_DECL(TerrainColorKegProperties);

enum class TerrainFunction {
    NOISE,
    RIDGED_NOISE,
    ABS_NOISE,
    SQUARED_NOISE,
    CUBED_NOISE
};
KEG_ENUM_DECL(TerrainFunction);

enum class TerrainOp {
    ADD = 0,
    SUB,
    MUL,
    DIV
};
KEG_ENUM_DECL(TerrainOp);

struct TerrainFuncKegProperties {
    TerrainFunction func = TerrainFunction::NOISE;
    TerrainOp op = TerrainOp::ADD;
    int octaves = 1;
    f32 persistence = 1.0f;
    f32 frequency = 1.0f;
    f32 low = -1.0f;
    f32 high = 1.0f;
    f32v2 clamp = f32v2(0.0f);
    Array<TerrainFuncKegProperties> children;
};
KEG_TYPE_DECL(TerrainFuncKegProperties);

struct NoiseBase {
    f32 base = 0.0f;
    Array<TerrainFuncKegProperties> funcs;
};
KEG_TYPE_DECL(NoiseBase);

struct PlanetGenData {
    vg::Texture terrainColorMap = 0;
    vg::Texture liquidColorMap = 0;
    vg::Texture terrainTexture = 0;
    vg::Texture liquidTexture = 0;
    color3 liquidTint = color3(255, 255, 255);
    color3 terrainTint = color3(255, 255, 255);
    f32 liquidDepthScale = 1000.0f;
    f32 liquidFreezeTemp = -1.0f;
    f32 tempLatitudeFalloff = 0.0f;
    f32 tempHeightFalloff = 0.0f;
    f32 humLatitudeFalloff = 0.0f;
    f32 humHeightFalloff = 0.0f;
    VGTexture biomeArrayTexture = 0;
    VGTexture baseBiomeLookupTexture = 0;
    std::vector<Biome> biomes;
    std::vector<BlockLayer> blockLayers;
    ui32 liquidBlock = 0;
    ui32 surfaceBlock = 0;
    vg::GLProgram* program = nullptr;
    f64 radius = 0.0;

    NoiseBase baseTerrainFuncs;
    NoiseBase tempTerrainFuncs;
    NoiseBase humTerrainFuncs;

    std::map <nString, ui32> blockColorMapLookupTable; ///< For looking up the index for the block color maps
    std::vector <color3*> blockColorMaps; ///< Storage for the block color maps
};

#endif // PlanetData_h__
