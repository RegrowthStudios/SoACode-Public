#include "stdafx.h"
#include "BlockPack.h"

BlockPack::BlockPack() :
    onBlockAddition(this) {
    
    Block defaults[2];
    { // Create "None" block
        Block& b = defaults[0];
        b.name = "None";
        b.allowLight = true;
        b.collide = false;
        b.floatingAction = 0;
        b.meshType = MeshType::NONE;
        b.occlude = BlockOcclusion::NONE;
        b.isSupportive = false;
        b.blockLight = false;
        b.useable = true;
    }
    { // Create "Unknown" block
        Block& b = defaults[1];
        b.name = "Unknown";
    }

    // Add default blocks
    append(defaults, 2);
}

void BlockPack::append(const Block* blocks, size_t n) {
    Block* block;
    size_t index;
    for (size_t i = 0; i < n; i++) {
        if (hasBlock(blocks[i].name, &block)) {
            index = block->ID;

            // Overwrite previous block
            *block = blocks[i];
        } else {
            index = m_blocks.size();

            // Add a new block
            m_blockList.push_back(blocks[i]);
            block = &m_blockList.back();
        }

        // Set the correct index
        block->ID = index;
        m_blocks[block->name] = index;
        onBlockAddition(block->ID);
    }
}

BlockPack Blocks;