///
/// Biome.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Provides the biome implementation.
///
#pragma once

#ifndef Biome_h__
#define Biome_h__

#include "Noise.h"
#include "Flora.h"

struct PlanetGenData;

// TODO(Ben): Also support L-system trees.
struct BiomeTree {
    NoiseBase chance;
    NTreeType* data = nullptr;
    FloraID id = FLORA_ID_NONE;
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
    FloraType* data = nullptr;
    FloraID id = FLORA_ID_NONE;
};

struct BlockLayer {
    ui32 start;
    ui32 width;
    ui16 block = 0;
    ui16 surfaceTransform = 0; ///< This block is used on surface
};

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
    Biome():id("default"), displayName("Default"), mapColor(255, 255, 255), genData(nullptr){}

    BiomeID id;
    nString displayName;
    ColorRGB8 mapColor; ///< For debugging and lookups
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
    const PlanetGenData* genData;
};

static const Biome DEFAULT_BIOME;

#endif // Biome_h__
