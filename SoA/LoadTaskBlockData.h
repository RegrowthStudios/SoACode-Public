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
        iom.setSearchDirectory("Data/Blocks/");
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
