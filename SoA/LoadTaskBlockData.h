#pragma once
#include "BlockPack.h"
#include "BlockLoader.h"
#include "Errors.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "LoadMonitor.h"
#include "Player.h"

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

        Player* player = GameManager::player;

        // Initialize objects
        for (size_t i = 1; i < Blocks.size(); i++){
            if (Blocks[i].active && !(i >= LOWWATER && i < FULLWATER || i > FULLWATER)){
                if (ObjectList[i] != NULL){
                    printf("Object ID %d already taken by %s. Requested by %s.\n", i, ObjectList[i]->name.c_str(), Blocks[i].name.c_str());
                    int a;
                    std::cin >> a;
                    exit(198);
                }
                ObjectList[i] = new Item(i, 1, Blocks[i].name, ITEM_BLOCK, 0, 0, 0);
                player->inventory.push_back(new Item(i, 9999999, Blocks[i].name, ITEM_BLOCK, 0, 0, 0));
            }
        }

        { // For use in pressure explosions
            Block visitedNode = Blocks[0];
            visitedNode.name = "Visited Node";
            Blocks.append(&visitedNode, 1);
            VISITED_NODE = Blocks["Visited Node"].ID;
        }

        findIDs(&Blocks);
    }
};