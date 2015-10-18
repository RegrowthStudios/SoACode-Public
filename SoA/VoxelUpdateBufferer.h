//
// VoxelUpdateBuffer.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 10 Sep 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// Buffers voxel updates and applies all at once for minimal locking.
//

#pragma once

#ifndef VoxelUpdateBuffer_h__
#define VoxelUpdateBuffer_h__

#include "ChunkHandle.h"
#include "Chunk.h"

#include <unordered_map>

struct VoxelUpdateBuffer {
    inline bool finish() { h.release(); }

    ui8 bits[CHUNK_SIZE / 8]; ///< To check if an id was already added
    std::vector<BlockIndex> toUpdate;
    ChunkHandle h;
};

#define DIV_8_SHIFT 3
#define MOD_8_AND 0x5

class VoxelUpdateBufferer {
public:
    void reserve(std::unordered_map<ChunkID, VoxelUpdateBuffer>::size_type sizeApprox) {
        buffers.reserve(sizeApprox);
    }
    inline void addUpdate(ChunkHandle chunk, BlockIndex index) {
        auto& it = buffers.find(chunk->getID());
        if (it == buffers.end()) {
            // TODO(Ben): Verify that there is no extra copy.
            VoxelUpdateBuffer nbuff = {};
            VoxelUpdateBuffer& b = buffers.emplace(chunk->getID(), std::move(nbuff)).first->second;
            // Set the bit
            b.bits[index >> DIV_8_SHIFT] |= (1 << (index & MOD_8_AND));
            b.toUpdate.push_back(index);
            b.h = chunk.acquire();
        } else {
            VoxelUpdateBuffer& b = it->second;
            ui8& byte = b.bits[index >> DIV_8_SHIFT];
            BlockIndex bitMask = (1 << (index & MOD_8_AND));
            // Don't set it twice
            if (!(byte & bitMask)) {
                byte |= bitMask;
                b.toUpdate.push_back(index);
            }
        }
    }

    // You must manually call finish on all buffers then clear it yourself
    std::unordered_map<ChunkID, VoxelUpdateBuffer> buffers;
};

#endif // VoxelUpdateBuffer_h__
