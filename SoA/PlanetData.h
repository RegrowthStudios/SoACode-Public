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
#include <Vorb/graphics/GLProgram.h>

#include "Biome.h"

DECL_VG(class GLProgram; class BitmapResource);

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

enum class TerrainStage {
    NOISE,
    RIDGED_NOISE,
    ABS_NOISE,
    SQUARED_NOISE,
    CUBED_NOISE,
    CELLULAR_NOISE,
    CONSTANT,
    PASS_THROUGH
};
KEG_ENUM_DECL(TerrainStage);

enum class TerrainOp {
    ADD = 0,
    SUB,
    MUL,
    DIV
};
KEG_ENUM_DECL(TerrainOp);

struct TerrainFuncKegProperties {
    TerrainStage func = TerrainStage::NOISE;
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

// For storing color maps
struct ColorMaps {
    std::map <nString, vg::BitmapResource*> colorMapTable; ///< For looking up block color maps by name
    std::vector <vg::BitmapResource*> colorMaps; ///< Storage for the block color maps
};

// Info about what blocks a planet needs
struct PlanetBlockInitInfo {
    std::vector<nString> blockLayerNames;
    nString liquidBlockName = "";
    nString surfaceBlockName = "";
};

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
    PlanetBlockInitInfo blockInfo;
    std::vector<Biome> biomes;
    std::vector<BlockLayer> blockLayers;
    ui32 liquidBlock = 0;
    ui32 surfaceBlock = 0;
    vg::GLProgram program;
    f64 radius = 0.0;

    NoiseBase baseTerrainFuncs;
    NoiseBase tempTerrainFuncs;
    NoiseBase humTerrainFuncs;

    static ColorMaps colorMaps;
    nString filePath;
};

#endif // PlanetData_h__
