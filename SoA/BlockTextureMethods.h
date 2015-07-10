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

#include <functional>

class ChunkMesher;
class BlockTextureLayer;
class BlockPack;

typedef ui32 BlockTextureIndex;

class BlockTextureMethodParams {
public:

    void init(ChunkMesher* cm, i32 RightDir, i32 UpDir, i32 FrontDir, ui32 face, ui32 layerIndex) {
        chunkMesher = cm;
        rightDir = RightDir;
        upDir = UpDir;
        frontDir = FrontDir;
        faceIndex = face;
        this->layerIndex = layerIndex;
    }

    void set(const BlockTextureLayer* blockTextureLayer, ui32 typeIndex, ColorRGB8& Color) {
        blockTexInfo = blockTextureLayer;
        this->typeIndex = typeIndex;
        color = &Color;
    }

    const ChunkMesher* chunkMesher = nullptr;
    const BlockTextureLayer* blockTexInfo = nullptr;
    i32 rightDir;
    i32 upDir;
    i32 frontDir;
    ui32 faceIndex;
    ui32 layerIndex;
    ui32 typeIndex;
    ColorRGB8* color = nullptr;
};

typedef std::function <void(BlockTextureMethodParams& params, BlockTextureIndex& result)> BlockTextureFunc;

namespace BlockTextureMethods {
    inline void getDefaultTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result) { /* Do nothing */ };
    extern void getRandomTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result);
    extern void getFloraTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result);
    extern void getConnectedTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result);
    extern void getGrassTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result);
    extern void getVerticalTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result);
    extern void getHorizontalTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result);
}

#endif // BlockTextureMethods_h__