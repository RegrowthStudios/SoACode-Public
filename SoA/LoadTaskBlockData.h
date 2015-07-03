#pragma once
#include "BlockPack.h"
#include "BlockLoader.h"
#include "Errors.h"
#include "GameManager.h"
#include "LoadMonitor.h"
#include "BlockTextureLoader.h"
#include "LoadContext.h"

#include <Vorb/io/IOManager.h>

// This is hacky and temporary, it does way to much
class LoadTaskBlockData : public ILoadTask {
public:
    LoadTaskBlockData(BlockPack* blockPack, BlockTextureLoader* loader, StaticLoadContext* context) :
        blockPack(blockPack), loader(loader), context(context) {
        context->addAnticipatedWork(50, 0);
    }

    virtual void load() {

        loader->loadTextureData();

        // TODO(Ben): Put in state
        vio::IOManager iom;
        iom.setSearchDirectory("Data/Blocks/");
        // Load in .yml
        if (!BlockLoader::loadBlocks(iom, blockPack)) {
            pError("Failed to load Data/BlockData.yml");
            exit(123456);
        }
        context->addWorkCompleted(40);

        for (size_t i = 0; i < blockPack->size(); i++) {
            Block& b = blockPack->operator[](i);
            if (b.active) {
                loader->loadBlockTextures(b);
            }
        }
        context->addWorkCompleted(10);
        // Uncomment to Save in .yml
        BlockLoader::saveBlocks("Data/Blocks/SavedBlockData.yml", blockPack);

        //{ // For use in pressure explosions
        //    Block visitedNode = Blocks[0];
        //    visitedNode.name = "Visited Node";
        //    Blocks.append(&visitedNode, 1);
        //    VISITED_NODE = Blocks["Visited Node"].ID;
        //}

    }
    BlockPack* blockPack;
    BlockTextureLoader* loader;
    StaticLoadContext* context;
};
