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
    LiquidColorKegProperties():
        colorPath(""),
        texturePath(""),
        tint(255, 255, 255),
        depthScale(1000.0f),
        freezeTemp(-1.0f)
    {}

    nString colorPath;
    nString texturePath;
    ColorRGB8 tint;
    f32 depthScale;
    f32 freezeTemp;
};
KEG_TYPE_DECL(LiquidColorKegProperties);

struct TerrainColorKegProperties {
    TerrainColorKegProperties():
        colorPath(""),
        grassTexturePath(""),
        rockTexturePath(""),
        tint(255, 255, 255)
    {}

    nString colorPath;
    nString grassTexturePath;
    nString rockTexturePath;
    ColorRGB8 tint;
};
KEG_TYPE_DECL(TerrainColorKegProperties);

// Must match var names for TreeFruitProperties
struct FruitKegProperties {
    FruitKegProperties():
        flora(""),
        chance(0.0f)
    {}

    nString flora = "";
    f32v2 chance = f32v2(0.0f);
};

// Must match var names for TreeLeafProperties
struct LeafKegProperties {
    LeafKegProperties():
        type(TreeLeafType::NONE),
        block("none"),
        mushGillBlock("none"),
        mushCapBlock("none")
    {}

    TreeLeafType type;
    FruitKegProperties fruitProps;
    // Union based on type
    union {
        struct {
            i32v2 vRadius;
            i32v2 hRadius;
        } round;
        struct {
            i32v2 oRadius;
            i32v2 iRadius;
            i32v2 period;
        } pine;
        struct {
            i32v2 tvRadius;
            i32v2 thRadius;
            i32v2 bvRadius;
            i32v2 bhRadius;
            i32v2 bLength;
            i32v2 capWidth;
            i32v2 gillWidth;
            FloraInterpType interp;
        } mushroom;
    };
    // Don't put strings in unions
    nString block;
    nString mushGillBlock;
    nString mushCapBlock;
};

// Must match var names for TreeBranchProperties
struct BranchKegProperties {
    BranchKegProperties():
        coreWidth(0),
        barkWidth(0),
        widthFalloff(0.1f),
        branchChance(0.0f),
        angle(0.0f),
        subBranchAngle(0.0f),
        changeDirChance(0.0f),
        coreBlock(""),
        barkBlock("")
    {}

    i32v2 coreWidth;
    i32v2 barkWidth;
    f32v2 widthFalloff;
    f32v2 branchChance;
    f32v2 angle;
    f32v2 subBranchAngle;
    f32v2 changeDirChance;
    nString coreBlock;
    nString barkBlock;
    FruitKegProperties fruitProps;
    LeafKegProperties leafProps;
};

// Must match var names for TreeTrunkProperties
struct TrunkKegProperties {
    TrunkKegProperties():
        loc(0.0f),
        coreWidth(0),
        barkWidth(0),
        branchChance(0.0f),
        changeDirChance(0.0f),
        coreBlock(""),
        barkBlock(""),
        interp(FloraInterpType::HERMITE)
    {}

    f32 loc;
    i32v2 coreWidth;
    i32v2 barkWidth;
    f32v2 branchChance;
    f32v2 changeDirChance;
    i32v2 slope[2];
    nString coreBlock;
    nString barkBlock;
    FloraInterpType interp;
    FruitKegProperties fruitProps;
    LeafKegProperties leafProps;
    BranchKegProperties branchProps;
};

struct BranchVolumeKegProperties {
    BranchVolumeKegProperties():
        height(0),
        hRadius(0),
        vRadius(0),
        points(0)
    {}

    i32v2 height;
    i32v2 hRadius;
    i32v2 vRadius;
    i32v2 points;
};

// Must match var names for TreeData
struct TreeKegProperties {
    TreeKegProperties():
        id(""),
        height(0),
        branchPoints(0),
        branchStep(0),
        killMult(2),
        infRadius(0.0f)
    {}

    nString id;
    i32v2 height;
    i32v2 branchPoints;
    i32v2 branchStep;
    i32v2 killMult;
    f32v2 infRadius;
    std::vector<BranchVolumeKegProperties> branchVolumes;
    std::vector<TrunkKegProperties> trunkProps;
};

struct FloraKegProperties {
    FloraKegProperties():
        block(""),
        nextFlora(""),
        height(1),
        slope(0),
        dSlope(0),
        dir(FloraDir::UP)
    {}

    nString id;
    nString block;
    nString nextFlora;
    i32v2 height;
    i32v2 slope;
    i32v2 dSlope;
    FloraDir dir;
};

struct BlockLayerKegProperties {
    BlockLayerKegProperties():
        block(""),
        surface(""),
        width(0)
    {}

    nString block;
    nString surface;
    ui32 width;
};
KEG_TYPE_DECL(BlockLayerKegProperties);

// Info about what blocks a planet needs
struct PlanetBlockInitInfo {
    PlanetBlockInitInfo():
        liquidBlockName(""),
        surfaceBlockName("")
    {}

    std::map<const Biome*, std::vector<BiomeFloraKegProperties>> biomeFlora;
    std::map<const Biome*, std::vector<BiomeTreeKegProperties>> biomeTrees;
    std::vector<TreeKegProperties> trees;
    std::vector<FloraKegProperties> flora;
    std::vector<BlockLayerKegProperties> blockLayers;
    nString liquidBlockName;
    nString surfaceBlockName;
};

struct PlanetGenData {
    PlanetGenData():
        terrainColorMap(0),
        liquidColorMap(0),
        grassTexture(0),
        rockTexture(0),
        liquidTexture(0),
        liquidTint(255, 255, 255),
        terrainTint(255, 255, 255),
        liquidDepthScale(1000.0f),
        liquidFreezeTemp(-1.0f),
        tempLatitudeFalloff(0.0f),
        tempHeightFalloff(0.0f),
        humLatitudeFalloff(0.0f),
        humHeightFalloff(0.0f),
        liquidBlock(0),
        surfaceBlock(0),
        radius(0.0)
    {}

    vg::Texture terrainColorMap;
    vg::Texture liquidColorMap;
    vg::Texture grassTexture;
    vg::Texture rockTexture;
    vg::Texture liquidTexture;
    vg::BitmapResource terrainColorPixels;
    vg::BitmapResource liquidColorPixels;
    color3 liquidTint;
    color3 terrainTint;
    f32 liquidDepthScale;
    f32 liquidFreezeTemp;
    f32 tempLatitudeFalloff;
    f32 tempHeightFalloff;
    f32 humLatitudeFalloff;
    f32 humHeightFalloff;
    PlanetBlockInitInfo blockInfo;
    std::vector<BlockLayer> blockLayers;
    ui32 liquidBlock;
    ui32 surfaceBlock;
    f64 radius;

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
