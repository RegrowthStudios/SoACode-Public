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

// Must match var names for TreeFruitProperties
struct FruitKegProperties {
    nString flora = "";
    f32v2 chance = f32v2(0.0f);
};

// Must match var names for TreeLeafProperties
struct LeafKegProperties {
    TreeLeafType type = TreeLeafType::NONE;
    FruitKegProperties fruitProps;
    // Union based on type
    union {
        UNIONIZE(struct {
            i32v2 vRadius;
            i32v2 hRadius;
        } round;);
        UNIONIZE(struct {
            i32v2 oRadius;
            i32v2 iRadius;
            i32v2 period;
        } pine;);
        UNIONIZE(struct {
            i32v2 tvRadius;
            i32v2 thRadius;
            i32v2 bvRadius;
            i32v2 bhRadius;
            i32v2 bLength;
            i32v2 capWidth;
            i32v2 gillWidth;
            FloraInterpType interp;
        } mushroom;);
    };
    // Don't put strings in unions
    nString block = "none";
    nString mushGillBlock = "none";
    nString mushCapBlock = "none";
};

// Must match var names for TreeBranchProperties
struct BranchKegProperties {
    BranchKegProperties() {
        segments[0] = i32v2(0);
        segments[1] = i32v2(0);
    }
    i32v2 coreWidth = i32v2(0);
    i32v2 barkWidth = i32v2(0);
    i32v2 length = i32v2(0);
    f32v2 branchChance = f32v2(0.0f);
    f32v2 angle = f32v2(0.0f);
    i32v2 segments[2];
    f32 endSizeMult = 0.0f;
    nString coreBlock = "";
    nString barkBlock = "";
    FruitKegProperties fruitProps;
    LeafKegProperties leafProps;
};

// Must match var names for TreeTrunkProperties
struct TrunkKegProperties {
    f32 loc = 0.0f;
    i32v2 coreWidth = i32v2(0);
    i32v2 barkWidth = i32v2(0);
    f32v2 branchChance = f32v2(0.0f);
    f32v2 changeDirChance = f32v2(0.0f);
    i32v2 slope[2];
    nString coreBlock = "";
    nString barkBlock = "";
    FloraInterpType interp = FloraInterpType::HERMITE;
    FruitKegProperties fruitProps;
    LeafKegProperties leafProps;
    BranchKegProperties branchProps;
};

struct BranchVolumeKegProperties {
    i32v2 height = i32v2(0);
    i32v2 hRadius = i32v2(0);
    i32v2 vRadius = i32v2(0);
    i32v2 points = i32v2(0);
};

// Must match var names for TreeData
struct TreeKegProperties {
    nString id = "";
    i32v2 height = i32v2(0);
    i32v2 branchPoints = i32v2(0);
    i32v2 branchStep = i32v2(0);
    i32v2 killMult = i32v2(2);
    f32v2 infRadius = f32v2(0.0f);
    std::vector<BranchVolumeKegProperties> branchVolumes;
    std::vector<TrunkKegProperties> trunkProps;
};

struct FloraKegProperties {
    nString id;
    nString block = "";
    nString nextFlora = "";
    i32v2 height = i32v2(1);
    i32v2 slope = i32v2(0);
    i32v2 dSlope = i32v2(0);
    FloraDir dir = FloraDir::UP;
};

struct BlockLayerKegProperties {
    nString block = "";
    nString surface = "";
    ui32 width = 0;
};
KEG_TYPE_DECL(BlockLayerKegProperties);

// Info about what blocks a planet needs
struct PlanetBlockInitInfo {
    std::map<const Biome*, std::vector<BiomeFloraKegProperties>> biomeFlora;
    std::map<const Biome*, std::vector<BiomeTreeKegProperties>> biomeTrees;
    std::vector<TreeKegProperties> trees;
    std::vector<FloraKegProperties> flora;
    std::vector<BlockLayerKegProperties> blockLayers;
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
    /* Flora and Trees                                                      */
    /************************************************************************/
    std::vector<FloraType> flora;
    std::map<nString, ui32> floraMap;
    std::vector<NTreeType> trees;
    std::map<nString, ui32> treeMap;

    /************************************************************************/
    /* Biomes                                                               */
    /************************************************************************/
    const Biome* baseBiomeLookup[BIOME_MAP_WIDTH][BIOME_MAP_WIDTH];
    std::vector<BiomeInfluence> baseBiomeInfluenceMap[BIOME_MAP_WIDTH][BIOME_MAP_WIDTH];
    std::vector<Biome> biomes; ///< Biome object storage. DON'T EVER RESIZE AFTER GEN.

    nString terrainFilePath;
};

#endif // PlanetData_h__
