#include "stdafx.h"
#include "BlockPack.h"

BlockPack::BlockPack() :
    onBlockAddition(this) {
    
    { // Create "None" block
        Block b;
        b.sID = "none";
        b.name = "None";
        b.allowLight = true;
        b.collide = false;
        b.floatingAction = 0;
        b.meshType = MeshType::NONE;
        b.occlude = BlockOcclusion::NONE;
        b.isSupportive = false;
        b.blockLight = false;
        b.useable = true;
        append(b);
    }
    { // Create "Unknown" block
        Block b;
        b.sID = "Unknown";
        b.name = "Unknown";
        append(b);
    }
}

BlockID BlockPack::append(Block& block) {
    const Block* curBlock;
    BlockID rv;
    if (curBlock = hasBlock(block.sID)) {
        rv = curBlock->ID;
        block.ID = rv;
        // Overwrite block
        *const_cast<Block*>(curBlock) = block;
    } else {
        rv = m_blockMap.size();
        block.ID = rv;
        // Add a new block
        m_blockList.push_back(block);
        // Set the correct index
        m_blockMap[block.sID] = rv;
    }
    onBlockAddition(block.ID);
    return rv;
}

void BlockPack::reserveID(const BlockIdentifier& sid, const BlockID& id) {
    if (id >= m_blockList.size()) m_blockList.resize(id + 1);
    m_blockMap[sid] = id;
    m_blockList[id].ID = id;
}
