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
    nString block;
};
KEG_TYPE_DECL(BlockLayer);

class Biome {
public:
    nString displayName;
    ColorRGB8 mapColor;
    Array<BlockLayer> blockLayers;
};
KEG_TYPE_DECL(Biome);

#endif // Biome_h__
