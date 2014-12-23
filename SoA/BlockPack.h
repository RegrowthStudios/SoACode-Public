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

/// A container for blocks
class BlockPack {
public:
    /// Constructor which adds default none and unknown blocks
    BlockPack();

    /// Add a series of blocks into this pack
    /// @param blocks: Pointer to block array
    /// @param n: Number of blocks
    void append(const Block* blocks, size_t n);

    /// 
    /// @param index: Block's index identifier
    /// @param block: 
    /// @return True if block exists
    bool hasBlock(const size_t& index, Block** block = nullptr) {
        if (index < 0 || index >= m_blockList.size()) {
            if (block) *block = nullptr;
            return false;
        } else {
            if (block) *block = &m_blockList[index];
            return true;
        }
    }
    bool hasBlock(const BlockIdentifier& id, Block** block = nullptr) {
        auto v = m_blocks.find(id);
        if (v == m_blocks.end()) {
            if (block) *block = nullptr;
            return false;
        } else {
            if (block) *block = &m_blockList[v->second];
            return true;
        }
    }

    /// @return Number of blocks in this pack
    size_t size() const {
        return m_blockList.size();
    }

    /************************************************************************/
    /* Block accessors                                                      */
    /************************************************************************/
    Block& operator[](const size_t& index) {
        return m_blockList[index];
    }
    const Block& operator[](const size_t& index) const {
        return m_blockList[index];
    }
    Block& operator[](const BlockIdentifier& id) {
        return m_blockList[m_blocks.at(id)];
    }
    const Block& operator[](const BlockIdentifier& id) const {
        return m_blockList[m_blocks.at(id)];
    }
private:
    std::unordered_map<BlockIdentifier, size_t> m_blocks;
    std::vector<Block> m_blockList;
};

// TODO: This will need to be removed
extern BlockPack Blocks;

#endif // BlockPack_h__