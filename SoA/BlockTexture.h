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

#define BASE_TYPE_INDEX 0
#define NORM_TYPE_INDEX 1
#define DISP_TYPE_INDEX 2

struct BlockColorMap;

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
    BlockTextureLayer() : index(0), normalIndex(0), dispIndex(0) {};

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

    // TODO(Ben): should it be ref color?
    inline void getBlockTextureMethodData(BlockTextureMethodParams& params, OUT color3& color, OUT BlockTextureMethodData& data) const {
        data.index = this->index;
        getTextureMethodData(params, BASE_TYPE_INDEX, color, data);
    }
    inline void getNormalTextureMethodData(BlockTextureMethodParams& params, OUT color3& color, OUT BlockTextureMethodData& data) const {
        data.index = this->normalIndex;
        return getTextureMethodData(params, NORM_TYPE_INDEX, color, data);
    }
    inline void getDispTextureMethodData(BlockTextureMethodParams& params, OUT color3& color, OUT  BlockTextureMethodData& data) const {
        data.index = this->dispIndex;
        return getTextureMethodData(params, DISP_TYPE_INDEX, color, data);
    }
    inline void getTextureMethodData(BlockTextureMethodParams& params, ui32 typeIndex, OUT color3& color, BlockTextureMethodData& data) const {
        params.set(this, typeIndex, color);
        blockTextureFunc(params, data);
    }

    void getFinalColor(OUT color3& color, ui8 temperature, ui8 rainfall, ui32 altColor) const;

    ConnectedTextureMethods method = ConnectedTextureMethods::NONE;
    ui8v2 size = ui8v2(1);
    ConnectedTextureSymmetry symmetry = ConnectedTextureSymmetry::NONE;
    ConnectedTextureReducedMethod reducedMethod = ConnectedTextureReducedMethod::NONE;
    BlockColorMap* colorMap = nullptr;
    color3 averageColor = color3(255, 255, 255); // Average texture color combined with color (for terrain)
    color3 color = color3(255, 255, 255);
    ui32 floraHeight = 0;
    Array<i32> weights;
    ui32 totalWeight = 0;
    ui32 numTiles = 1;
    union {
        struct {
            BlockTextureIndex index;
            BlockTextureIndex normalIndex;
            BlockTextureIndex dispIndex;
        };
        BlockTextureIndex indices[3];
    };
    bool innerSeams = false;
    bool transparency = false;
    nString path = "";
    nString normalPath = "";
    nString dispPath = "";
    nString colorMapPath = "";
    BlockTextureFunc blockTextureFunc = BlockTextureMethods::getDefaultTextureIndex;

    /// "less than" operator for inserting into sets in TexturePackLoader
    // TODO(Ben): Are these operators needed?
    bool operator<(const BlockTextureLayer& b) const;
    bool operator==(const BlockTextureLayer& b) const {
        return method == b.method && size == b.size && symmetry == b.symmetry &&
            reducedMethod == b.reducedMethod && colorMap == b.colorMap &&
            color == b.color && 
            averageColor == b.averageColor && floraHeight == b.floraHeight &&
            totalWeight == b.totalWeight && numTiles == b.numTiles && index == b.index &&
            innerSeams == b.innerSeams && transparency == b.transparency && path == b.path;
    }
};

struct BlockTexture {
    union {
        struct {
            BlockTextureLayer base;
            BlockTextureLayer overlay;
        };
        UNIONIZE(BlockTextureLayer layers[2]);
    };
    BlendType blendMode = BlendType::ALPHA;
};
KEG_TYPE_DECL(BlockTexture);

#endif // BlockTexture_h__
