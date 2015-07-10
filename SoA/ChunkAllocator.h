///
/// ChunkAllocator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 9 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Paged memory management system for chunks
///

#pragma once

#ifndef ChunkAllocator_h__
#define ChunkAllocator_h__

#include "Chunk.h"
#include "Constants.h"
#include <vector>
#include <Vorb/FixedSizeArrayRecycler.hpp>


class ChunkAllocator {
public:
    virtual Chunk* getNewChunk() = 0;
    virtual void freeChunk(Chunk* chunk) = 0;
};

class PagedChunkAllocator : public ChunkAllocator {
public:
    friend class SphericalVoxelComponentUpdater;

    PagedChunkAllocator();
    ~PagedChunkAllocator();

    /// Gets a new chunk ID
    Chunk* getNewChunk() override;
    /// Frees a chunk
    void freeChunk(Chunk* chunk) override;
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
