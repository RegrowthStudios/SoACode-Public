//
// ChunkAllocator.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 9 Jun 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// Paged memory management system for chunks.
//

#pragma once

#ifndef ChunkAllocator_h__
#define ChunkAllocator_h__

#include <Vorb/FixedSizeArrayRecycler.hpp>

#include "Chunk.h"
#include "Constants.h"

/*! @brief The chunk allocator.
 */
class PagedChunkAllocator {
    friend class SphericalVoxelComponentUpdater;
public:
    PagedChunkAllocator();
    ~PagedChunkAllocator();

    typedef void(*MemoryFormatter)(Chunk* chunk, void* memory, void* userData);
    
    //void appendExtraSize(size_t s, MemoryFormatter fConstructor);

    /// Gets a new chunk ID
    Chunk* alloc();
    /// Frees a chunk
    void free(Chunk* chunk);
protected:
    static const size_t CHUNK_PAGE_SIZE = 2048;
    struct ChunkPage {
        Chunk chunks[CHUNK_PAGE_SIZE];
    };

    std::vector<Chunk*> m_freeChunks; ///< List of inactive chunks
    std::vector<ChunkPage*> m_chunkPages; ///< All pages
    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16> m_shortFixedSizeArrayRecycler; ///< For recycling voxel data
};

#endif // ChunkAllocator_h__
