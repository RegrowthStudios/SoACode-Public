//
// ChunkAccessor.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 30 Jul 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// Fires events for chunk access
//

#pragma once

#ifndef ChunkAccessor_h__
#define ChunkAccessor_h__

#include "Chunk.h"
#include "ChunkHandle.h"

class ChunkAccessor {
    friend class ChunkHandle;
public:
    void init(PagedChunkAllocator* allocator);
    void destroy();

    ChunkHandle acquire(ChunkID id);
private:
    ChunkHandle acquire(ChunkHandle& chunk);
    void release(ChunkHandle& chunk);

    ChunkHandle safeAdd(ChunkID id, bool& wasOld);
    void safeRemove(ChunkID id);

    std::mutex m_lckLookup;
    std::unordered_map<ChunkID, ChunkHandle> m_chunkLookup;
    PagedChunkAllocator* m_allocator = nullptr;
};

#endif // ChunkAccessor_h__
