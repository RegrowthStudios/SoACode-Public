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

struct BlockTextureMethodData {
    BlockTextureIndex index;
    ui8v2 size;
};

typedef std::function <void(BlockTextureMethodParams& params, BlockTextureMethodData& result)> BlockTextureFunc;

namespace BlockTextureMethods {
    void getDefaultTextureIndex(BlockTextureMethodParams& params, BlockTextureMethodData& result);
    void getRandomTextureIndex(BlockTextureMethodParams& params, BlockTextureMethodData& result);
    void getFloraTextureIndex(BlockTextureMethodParams& params, BlockTextureMethodData& result);
    void getConnectedTextureIndex(BlockTextureMethodParams& params, BlockTextureMethodData& result);
    void getGrassTextureIndex(BlockTextureMethodParams& params, BlockTextureMethodData& result);
    void getVerticalTextureIndex(BlockTextureMethodParams& params, BlockTextureMethodData& result);
    void getHorizontalTextureIndex(BlockTextureMethodParams& params, BlockTextureMethodData& result);
}

#endif // BlockTextureMethods_h__
