#pragma once
#include "BlockPack.h"
#include "BlockLoader.h"
#include "Errors.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "LoadMonitor.h"

// This is hacky and temporary, it does way to much
class LoadTaskBlockData : public ILoadTask {
    virtual void load() {
        // Load in .yml
        if (!BlockLoader::loadBlocks("Data/BlockData.yml", &Blocks)) {
            pError("Failed to load Data/BlockData.yml");
            exit(123456);
        }

        // Uncomment to Save in .yml
        //BlockLoader::saveBlocks("Data/SavedBlockData.yml");

        { // For use in pressure explosions
            Block visitedNode = Blocks[0];
            visitedNode.name = "Visited Node";
            Blocks.append(&visitedNode, 1);
            VISITED_NODE = Blocks["Visited Node"].ID;
        }

        findIDs(&Blocks);
    }
};