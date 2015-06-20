///
/// BlockTexture.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 16 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Texture information for blocks
///

#pragma once

#ifndef BlockTexture_h__
#define BlockTexture_h__

#include <Vorb/io/Keg.h>

#include "BlockTextureMethods.h"

enum class ConnectedTextureMethods {
    NONE,
    CONNECTED,
    HORIZONTAL,
    VERTICAL,
    GRASS,
    REPEAT,
    RANDOM,
    FLORA
};
KEG_ENUM_DECL(ConnectedTextureMethods);

enum class ConnectedTextureSymmetry {
    NONE,
    OPPOSITE,
    ALL
};
KEG_ENUM_DECL(ConnectedTextureSymmetry);

enum class ConnectedTextureReducedMethod {
    NONE,
    TOP,
    BOTTOM
};
KEG_ENUM_DECL(ConnectedTextureReducedMethod);

enum class BlendType {
    ALPHA,
    ADD,
    SUBTRACT,
    MULTIPLY
};
KEG_ENUM_DECL(BlendType);

class BlockTextureLayer {
public:
    static ui32 getFloraRows(ui32 floraMaxHeight) {
        return (floraMaxHeight * floraMaxHeight + floraMaxHeight) / 2;
    }

    // Sets the texture func based on the method
    // needs to have the method
    void initBlockTextureFunc() {
        switch (method) {
            case ConnectedTextureMethods::CONNECTED:
                blockTextureFunc = BlockTextureMethods::getConnectedTextureIndex;
                break;
            case ConnectedTextureMethods::RANDOM:
                blockTextureFunc = BlockTextureMethods::getRandomTextureIndex;
                break;
            case ConnectedTextureMethods::GRASS:
                blockTextureFunc = BlockTextureMethods::getGrassTextureIndex;
                break;
            case ConnectedTextureMethods::HORIZONTAL:
                blockTextureFunc = BlockTextureMethods::getHorizontalTextureIndex;
                break;
            case ConnectedTextureMethods::VERTICAL:
                blockTextureFunc = BlockTextureMethods::getVerticalTextureIndex;
                break;
            case ConnectedTextureMethods::FLORA:
                blockTextureFunc = BlockTextureMethods::getFloraTextureIndex;
                break;
            default:
                break;
        }
    }

    BlockTextureIndex getBlockTextureIndex(BlockTextureMethodParams& params, ColorRGB8 color) const {
        BlockTextureIndex index = this->index;
        params.set(this, color);
        blockTextureFunc(params, index);
        return index;
    }

    ConnectedTextureMethods method = ConnectedTextureMethods::NONE;
    ui32v2 size = ui32v2(1);
    ConnectedTextureSymmetry symmetry = ConnectedTextureSymmetry::NONE;
    ConnectedTextureReducedMethod reducedMethod = ConnectedTextureReducedMethod::NONE;
    nString useMapColor = "";
    color4 averageColor = color4(0, 0, 0, 0); // Lowest mipmap pixel
    ui32 floraHeight = 0;
    Array<i32> weights;
    ui32 totalWeight = 0;
    ui32 numTiles = 1;
    BlockTextureIndex index = 0;
    bool innerSeams = false;
    bool transparency = false;
    nString path = "";
    BlockTextureFunc blockTextureFunc = BlockTextureMethods::getDefaultTextureIndex;

    /// "less than" operator for inserting into sets in TexturePackLoader
    bool operator<(const BlockTextureLayer& b) const;
};
KEG_TYPE_DECL(BlockTextureLayer);

struct BlockTexture {
    BlockTextureLayer base;
    BlockTextureLayer overlay;
    BlendType blendMode = BlendType::ALPHA;
};
KEG_TYPE_DECL(BlockTexture);

#endif // BlockTexture_h__
