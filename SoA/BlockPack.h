///
/// BlockPack.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 23 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Container for block data
///

#pragma once

#ifndef BlockPack_h__
#define BlockPack_h__

#include "BlockData.h"

class BlockPack {
public:
    void append(const Block* blocks, size_t n);
private:
    std::unordered_map<BlockIdentifier, Block*> m_blocks;
    std::vector<Block> m_blockList;
};

#endif // BlockPack_h__