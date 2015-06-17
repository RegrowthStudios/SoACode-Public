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

#include <Vorb/Events.hpp>
#include <Vorb/graphics/Texture.h>

#include "BlockData.h"

typedef ui16 BlockID;

/// A container for blocks
class BlockPack {
public:
    /// Constructor which adds default none and unknown blocks
    BlockPack();

    /// Add a block to the pack, and overwrite a block of the same BlockIdentifier
    /// Will invalidate existing Block* pointers. Store BlockIDs instead.
    BlockID append(Block& block);

    void reserveID(const BlockIdentifier& sid, const BlockID& id);

    /// Note that the returned pointer becomes invalidated after an append call
    /// @return nullptr if block doesn't exist
    Block* hasBlock(const BlockID& id) {
        if (id >= m_blockList.size()) {
            return nullptr;
        } else {
            return &m_blockList[id];
        }
    }
    Block* hasBlock(const BlockIdentifier& sid) {
        auto v = m_blockMap.find(sid);
        if (v == m_blockMap.end()) {
            return nullptr;
        } else {
            return &m_blockList[v->second];
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
    Block& operator[](const BlockIdentifier& sid) {
        return m_blockList[m_blockMap.at(sid)];
    }
    const Block& operator[](const BlockIdentifier& sid) const {
        return m_blockList[m_blockMap.at(sid)];
    }

    const std::unordered_map<BlockIdentifier, ui16>& getBlockMap() const { return m_blockMap; }
    const std::vector<Block>& getBlockList() const { return m_blockList; }

    Event<ui16> onBlockAddition; ///< Signaled when a block is loaded
    vg::Texture texture; // TODO(Ben): Move?
private:
    std::unordered_map<BlockIdentifier, ui16> m_blockMap; ///< Blocks indices organized by identifiers
    std::vector<Block> m_blockList; ///< Block data list
};

#endif // BlockPack_h__
