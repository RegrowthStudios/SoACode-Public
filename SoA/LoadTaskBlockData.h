#pragma once
#include "BlockPack.h"
#include "BlockLoader.h"
#include "Errors.h"
#include "GameManager.h"
#include "LoadMonitor.h"

#include <Vorb/io/IOManager.h>

// This is hacky and temporary, it does way to much
class LoadTaskBlockData : public ILoadTask {
public:
    LoadTaskBlockData(BlockPack* blockPack) : m_blockPack(blockPack) {}

    virtual void load() {
        // TODO(Ben): Put in state
        vio::IOManager iom;
        iom.setSearchDirectory("Data/Blocks");
        // Load in .yml
        if (!BlockLoader::loadBlocks(iom, m_blockPack)) {
            pError("Failed to load Data/BlockData.yml");
            exit(123456);
        }

        // Uncomment to Save in .yml
        //BlockLoader::saveBlocks("Data/SavedBlockData.yml");

        //{ // For use in pressure explosions
        //    Block visitedNode = Blocks[0];
        //    visitedNode.name = "Visited Node";
        //    Blocks.append(&visitedNode, 1);
        //    VISITED_NODE = Blocks["Visited Node"].ID;
        //}

    }

    BlockPack* m_blockPack;
};

class BlockTextureLayer {
public:
    static ui32 getFloraRows(ui32 floraMaxHeight) {
        return (floraMaxHeight * floraMaxHeight + floraMaxHeight) / 2;
    }

    // Sets the texture funct based on the method
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

    i32 getBlockTextureIndex(BlockTextureMethodParams& params, ColorRGB8 color) const {
        i32 index = textureIndex;
        params.set(this, color);
        blockTextureFunc(params, index);
        return index;
    }

    ConnectedTextureMethods method = ConnectedTextureMethods::NONE;
    ui8v2 size = ui8v2(1);
    ConnectedTextureSymmetry symmetry = ConnectedTextureSymmetry::NONE;
    ConnectedTextureReducedMethod reducedMethod = ConnectedTextureReducedMethod::NONE;
    nString useMapColor = "";
    vg::BitmapResource* colorMap = nullptr;
    ui32 floraHeight = 0;
    Array<i32> weights;
    i32 totalWeight = 0;
    i32 numTiles = 1;
    i32 textureIndex = 0;
    bool innerSeams = false;
    bool transparency = false;
    nString path = ""; //< TODO(Ben): In BlockTexture instead?
    BlockTextureFunc blockTextureFunc = BlockTextureMethods::getDefaultTextureIndex;

    /// "less than" operator for inserting into sets in TexturePackLoader
    bool operator<(const BlockTextureLayer& b) const;
};
KEG_TYPE_DECL(BlockTextureLayer);

class BlockTexture {
public:
    BlockTexture() {};
    BlockTexture(const BlockTextureLayer& b, const BlockTextureLayer& o, BlendType bt) :
        base(b), overlay(o), blendMode(bt) {
        // Empty
    }
    BlockTextureLayer base;
    BlockTextureLayer overlay;

    BlendType blendMode = BlendType::ALPHA;
};
KEG_TYPE_DECL(BlockTexture);