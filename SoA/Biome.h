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

struct BiomeTree {
public:
    f32 chance;
};

struct BiomeFlora {
    f32 chance;
    ui16 block = 0;
};
KEG_TYPE_DECL(BiomeFlora);

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

struct Biome {
    BiomeID id = "default";
    nString displayName = "Default";
    ColorRGB8 mapColor = ColorRGB8(255, 255, 255); ///< For debugging and lookups
    std::vector<BlockLayer> blockLayers; ///< Overrides base layers
    std::vector<Biome*> children;
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
