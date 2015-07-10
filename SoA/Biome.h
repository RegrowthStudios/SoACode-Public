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

#include "Noise.h"

class BiomeTree {
public:
    BiomeTree(f32 prob, i32 index) : probability(prob),
    treeIndex(index) {
    }
    f32 probability;
    i32 treeIndex;
};

class BiomeFlora {
public:
    BiomeFlora(f32 prob, i32 index) : probability(prob),
    floraIndex(index) {
    }
    f32 probability;
    i32 floraIndex;
};

struct BlockLayer {
    ui32 start;
    ui32 width;
    ui32 block = 0;
};
KEG_TYPE_DECL(BlockLayer);

#define BIOME_MAP_WIDTH 256

// TODO(Ben): Add more
enum class BiomeAxisType { HEIGHT, NOISE };
KEG_ENUM_DECL(BiomeAxisType);

typedef nString BiomeID;
struct Biome;

struct BiomeInfluence {
    BiomeInfluence() {};
    BiomeInfluence(const Biome* b, f32 weight) : b(b), weight(weight) {}
    const Biome* b;
    f32 weight;

    bool operator<(const BiomeInfluence& rhs) const {
        if (weight < rhs.weight) return true;
        return b < rhs.b;
    }
};

// TODO(Ben): Make the memory one contiguous block
typedef std::vector<std::vector<BiomeInfluence>> BiomeInfluenceMap;

struct Biome {
    BiomeID id = "default";
    nString displayName = "Default";
    ColorRGB8 mapColor = ColorRGB8(255, 255, 255); ///< For debugging and lookups
    std::vector<BlockLayer> blockLayers; ///< Overrides base layers
    BiomeInfluenceMap biomeMap; ///< Optional sub-biome map
    BiomeAxisType axisTypes[2];
    f32v2 heightScale; ///< Scales height for BIOME_AXIS_TYPE::HEIGHT
    NoiseBase biomeMapNoise; ///< For sub biome determination
    NoiseBase terrainNoise; ///< Modifies terrain directly
    NoiseBase xNoise;
    NoiseBase yNoise;
};

static const Biome DEFAULT_BIOME;

#endif // Biome_h__
