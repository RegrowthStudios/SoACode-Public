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
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/ImageIO.h>
#include <Vorb/graphics/Texture.h>
#include <Vorb/io/Keg.h>

#include "Noise.h"
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
    nString grassTexturePath = "";
    nString rockTexturePath = "";
    ColorRGB8 tint = ColorRGB8(255, 255, 255);
};
KEG_TYPE_DECL(TerrainColorKegProperties);

// Info about what blocks a planet needs
struct PlanetBlockInitInfo {
    std::vector<nString> blockLayerNames;
    nString liquidBlockName = "";
    nString surfaceBlockName = "";
};

struct PlanetGenData {
    vg::Texture terrainColorMap = 0;
    vg::Texture liquidColorMap = 0;
    vg::Texture grassTexture = 0;
    vg::Texture rockTexture = 0;
    vg::Texture liquidTexture = 0;
    vg::BitmapResource terrainColorPixels;
    vg::BitmapResource liquidColorPixels;
    color3 liquidTint = color3(255, 255, 255);
    color3 terrainTint = color3(255, 255, 255);
    f32 liquidDepthScale = 1000.0f;
    f32 liquidFreezeTemp = -1.0f;
    f32 tempLatitudeFalloff = 0.0f;
    f32 tempHeightFalloff = 0.0f;
    f32 humLatitudeFalloff = 0.0f;
    f32 humHeightFalloff = 0.0f;
    PlanetBlockInitInfo blockInfo;
    std::vector<BlockLayer> blockLayers;
    ui32 liquidBlock = 0;
    ui32 surfaceBlock = 0;
    f64 radius = 0.0;

    /************************************************************************/
    /* Base Noise                                                           */
    /************************************************************************/
    NoiseBase baseTerrainFuncs;
    NoiseBase tempTerrainFuncs;
    NoiseBase humTerrainFuncs;

    /************************************************************************/
    /* Biomes                                                               */
    /************************************************************************/
    const Biome* baseBiomeLookup[BIOME_MAP_WIDTH][BIOME_MAP_WIDTH];
    std::vector<BiomeInfluence> baseBiomeInfluenceMap[BIOME_MAP_WIDTH][BIOME_MAP_WIDTH];
    std::vector<Biome> biomes; ///< Biome object storage. DON'T EVER RESIZE AFTER GEN.

    nString filePath;
};

#endif // PlanetData_h__
