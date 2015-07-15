///
/// Biome.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Provides the biome implementation
///
#pragma once

#ifndef Biome_h__
#define Biome_h__

#include <Vorb/io/Keg.h>
#include <Vorb/utils.h>

#include "Noise.h"

#define FLORA_ID_NONE 0xFFFFu
// Both trees and flora share FloraID
typedef ui16 FloraID;

enum class TreeLeafType {
    NONE,
    ROUND,
    CLUSTER,
    PINE,
    MUSHROOM
};
KEG_ENUM_DECL(TreeLeafType);

struct TreeFruitProperties {
    FloraID flora = FLORA_ID_NONE;
    Range<f32> chance;
};

struct TreeLeafProperties {
    TreeLeafType type;
    TreeFruitProperties fruitProps;
    // Mutually exclusive union based on type
    union {
        UNIONIZE(struct {
            Range<ui16> radius;
            ui16 blockID;
        } round;);
        UNIONIZE(struct {
            Range<ui16> width;
            Range<ui16> height;
            ui16 blockID;
        } cluster;);
        UNIONIZE(struct {
            Range<ui16> thickness;
            ui16 blockID;
        } pine;);
        UNIONIZE(struct {
            Range<i16> lengthMod;
            Range<i16> curlLength;
            Range<i16> capThickness;
            Range<i16> gillThickness;
            ui16 gillBlockID;
            ui16 capBlockID;
        } mushroom;);
    };
};

struct TreeBranchProperties {
    Range<ui16> coreWidth;
    Range<ui16> barkWidth;
    Range<f32> branchChance;
    ui16 coreBlockID = 0;
    ui16 barkBlockID = 0;
    TreeFruitProperties fruitProps;
    TreeLeafProperties leafProps;
};

struct TreeTrunkProperties {
    Range<ui16> coreWidth;
    Range<ui16> barkWidth;
    Range<f32> branchChance;
    Range<Range<i32>> slope;
    ui16 coreBlockID = 0;
    ui16 barkBlockID = 0;
    TreeFruitProperties fruitProps;
    TreeLeafProperties leafProps;
    TreeBranchProperties branchProps;
};

struct TreeData {
    // All ranges are for scaling between baby tree and adult tree
    Range<ui16> heightRange;
    // Data points for trunk properties. Properties get interpolated between these from
    // base of tree to top of tree.
    std::vector<TreeTrunkProperties> trunkProps;
};

// TODO(Ben): Also support L-system trees.
struct BiomeTree {
    NoiseBase chance;
    TreeData* data = nullptr;
    FloraID id = FLORA_ID_NONE;
};

// Flora specification
struct FloraData {
    ui16 block;
};

struct BiomeFloraKegProperties {
    NoiseBase chance;
    nString id;
};
KEG_TYPE_DECL(BiomeFloraKegProperties);

struct BiomeTreeKegProperties {
    NoiseBase chance;
    nString id;
};
KEG_TYPE_DECL(BiomeTreeKegProperties);

// Unique flora instance
struct BiomeFlora {
    NoiseBase chance;
    FloraData data;
    FloraID id = FLORA_ID_NONE;
};

struct BlockLayer {
    ui32 start;
    ui32 width;
    ui32 block = 0;
};
KEG_TYPE_DECL(BlockLayer);

#define BIOME_MAP_WIDTH 256

typedef nString BiomeID;
struct Biome;

struct BiomeInfluence {
    BiomeInfluence() {};
    BiomeInfluence(const Biome* b, f32 weight) : b(b), weight(weight) {}
    const Biome* b;
    f32 weight;

    bool operator<(const BiomeInfluence& rhs) const {
        // Ignore weight on purpose. Only one BiomeInfluence per set or map!
        return b < rhs.b;
    }
};

// TODO(Ben): Make the memory one contiguous block
typedef std::vector<std::vector<BiomeInfluence>> BiomeInfluenceMap;

// TODO(Ben): Optimize the cache
struct Biome {
    BiomeID id = "default";
    nString displayName = "Default";
    ColorRGB8 mapColor = ColorRGB8(255, 255, 255); ///< For debugging and lookups
    std::vector<BlockLayer> blockLayers; ///< Overrides base layers
    std::vector<Biome*> children;
    std::vector<BiomeFlora> flora;
    std::vector<BiomeTree> trees;
    NoiseBase childNoise; ///< For sub biome determination
    NoiseBase terrainNoise; ///< Modifies terrain directly
    // Only applies to base biomes
    f64v2 heightRange;
    f64v2 heightScale;
    f64v2 noiseRange;
    f64v2 noiseScale;
};

static const Biome DEFAULT_BIOME;

#endif // Biome_h__
