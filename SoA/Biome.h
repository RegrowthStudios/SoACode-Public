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
enum class BIOME_AXIS_TYPE { HEIGHT, NOISE };

struct Biome {
    nString displayName;
    ColorRGB8 mapColor; ///< For debugging
    Array<BlockLayer> blockLayers; ///< Overrides base layers
    BiomeMap* biomeMap = nullptr; ///< Optional sub-biome map
    BIOME_AXIS_TYPE axisTypes[2];
    f32v2 heightScale; ///< Scales height for BIOME_AXIS_TYPE::HEIGHT
    NoiseBase terrainNoise; ///< Modifies terrain directly
    NoiseBase biomeMapNoise; ///< For sub biome determination
};
KEG_TYPE_DECL(Biome);

struct BiomeInfluence {
    Biome* biome;
    f32 influence; ///< For interpolation
};

// TODO(Ben): This could be super cache friendly with a bit of work
struct BiomeMap {
    std::vector<BiomeInfluence> biomes[BIOME_MAP_WIDTH][BIOME_MAP_WIDTH];
};

#endif // Biome_h__
