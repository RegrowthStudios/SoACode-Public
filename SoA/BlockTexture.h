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
    BlockTextureLayer() : method(ConnectedTextureMethods::NONE), size(1), symmetry(ConnectedTextureSymmetry::NONE),
    reducedMethod(ConnectedTextureReducedMethod::NONE), colorMap(nullptr), averageColor(255, 255, 255), color(255, 255, 255),
    floraHeight(0), totalWeight(0), numTiles(1), indices{0, 0, 0},  innerSeams(false), transparency(false), path(""), normalPath(""),
    dispPath(""), colorMapPath(""), blockTextureFunc(BlockTextureMethods::getDefaultTextureIndex)
        {}

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
        data.index = this->index.layer;
        getTextureMethodData(params, BASE_TYPE_INDEX, color, data);
    }
    inline void getNormalTextureMethodData(BlockTextureMethodParams& params, OUT color3& color, OUT BlockTextureMethodData& data) const {
        data.index = this->index.normal;
        return getTextureMethodData(params, NORM_TYPE_INDEX, color, data);
    }
    inline void getDispTextureMethodData(BlockTextureMethodParams& params, OUT color3& color, OUT  BlockTextureMethodData& data) const {
        data.index = this->index.disp;
        return getTextureMethodData(params, DISP_TYPE_INDEX, color, data);
    }
    inline void getTextureMethodData(BlockTextureMethodParams& params, ui32 typeIndex, OUT color3& color, BlockTextureMethodData& data) const {
        params.set(this, typeIndex, color);
        blockTextureFunc(params, data);
    }

    void getFinalColor(OUT color3& color, ui8 temperature, ui8 rainfall, ui32 altColor) const;

    ConnectedTextureMethods method;
    ui8v2 size;
    ConnectedTextureSymmetry symmetry;
    ConnectedTextureReducedMethod reducedMethod;
    BlockColorMap* colorMap;
    color3 averageColor; // Average texture color combined with color (for terrain)
    color3 color;
    ui32 floraHeight;
    Array<i32> weights;
    ui32 totalWeight;
    ui32 numTiles;
    union {
        struct {
            BlockTextureIndex layer;
            BlockTextureIndex normal;
            BlockTextureIndex disp;
        } index;
        BlockTextureIndex indices[3];
    };
    bool innerSeams;
    bool transparency;
    nString path;
    nString normalPath;
    nString dispPath;
    nString colorMapPath;
    BlockTextureFunc blockTextureFunc;

    /// "less than" operator for inserting into sets in TexturePackLoader
    // TODO(Ben): Are these operators needed?
    bool operator<(const BlockTextureLayer& b) const;
    bool operator==(const BlockTextureLayer& b) const {
        return method == b.method && size == b.size && symmetry == b.symmetry &&
            reducedMethod == b.reducedMethod && colorMap == b.colorMap &&
            color == b.color && 
            averageColor == b.averageColor && floraHeight == b.floraHeight &&
            totalWeight == b.totalWeight && numTiles == b.numTiles && index.layer == b.index.layer &&
            innerSeams == b.innerSeams && transparency == b.transparency && path == b.path;
    }
};

struct BlockTexture {
    BlockTexture(): layers(), blendMode(BlendType::ALPHA)
    {
    }
    //provide deconstructor because of union
    ~BlockTexture()
    {
        layers.base.BlockTextureLayer::~BlockTextureLayer();
        layers.overlay.BlockTextureLayer::~BlockTextureLayer();
    }
    
    struct {
        BlockTextureLayer base;
        BlockTextureLayer overlay;
    } layers;
    BlendType blendMode;
};
KEG_TYPE_DECL(BlockTexture);

#endif // BlockTexture_h__
