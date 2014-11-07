///
/// BlockTextureMethods.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 7 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// This file provides the texturing methods functions for block textures,
/// such as connected textures and random textures.
///

#pragma once

#ifndef BlockTextureMethods_h__
#define BlockTextureMethods_h__

#include "BlockData.h"
#include "ChunkMesh.h"

class BlockTextureMethodParams {
public:
    const MesherInfo* mesherInfo;
    const BlockTextureLayer* blockTexInfo;
    i32 rightDir;
    i32 upDir;
    i32 frontDir;
    ui32 offset;
    ColorRGB8* color;
};


typedef std::function <void(BlockTextureMethodParams& params, int& result)> blockTextureFunc;

namespace BlockTextureMethods {
    void getDefaultTextureIndex(BlockTextureMethodParams& params, int& result) { /* Do nothing */ };
    extern void getRandomTextureIndex(BlockTextureMethodParams& params, int& result);
    extern void getFloraTextureIndex(BlockTextureMethodParams& paramso, int& result);
    extern void getConnectedTextureIndex(BlockTextureMethodParams& params, int& result);
    extern void getGrassTextureIndex(BlockTextureMethodParams& params, int& result);
    extern void getVerticalTextureIndex(BlockTextureMethodParams& params, int& result);
    extern void getHorizontalTextureIndex(BlockTextureMethodParams& params, int& resultt);
}

#endif // BlockTextureMethods_h__