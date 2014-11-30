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

#include "stdafx.h"


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
    ui32 looseSoilDepth = 4;

    ui16 surfaceBlock = DIRTGRASS;
    ui16 underwaterBlock = SAND;
    ui16 beachBlock = SAND;
   
    f32 treeChance = 0.0f;

    nString name = "NO BIOME";
    nString fileName = "";

    std::vector <Biome*> childBiomes;
    std::vector <BiomeTree> possibleTrees;
    std::vector <BiomeFlora> possibleFlora;
};

#endif // Biome_h__
