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

#include <vector>
#include "Keg.h"

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

class Biome {
public:
    nString displayName;
    ColorRGB8 mapColor;
};
KEG_TYPE_DECL(Biome);

#endif // Biome_h__
