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

#include "Constants.h"
#include <vector>
#include <Vorb/FixedSizeArrayRecycler.hpp>

class NChunk;

class ChunkAllocator {
public:
    friend class SphericalVoxelComponentUpdater;

    ChunkAllocator();
    ~ChunkAllocator();

    /// Gets a new chunk ID
    NChunk* getNewChunk();
    /// Frees a chunk
    void freeChunk(NChunk* chunk);

    const std::vector<NChunk*>& getActiveChunks() const { return m_activeChunks; }
private:
    static const size_t CHUNK_PAGE_SIZE = 2048;
    struct ChunkPage {
        NChunk chunks[CHUNK_PAGE_SIZE];
    };

    // TODO(BEN): limit number of pages that can be allocated
    std::vector<NChunk*> m_activeChunks; ///< List of currently active chunks
    std::vector<NChunk*> m_freeChunks; ///< List of inactive chunks
    std::vector<ChunkPage*> m_chunkPages; ///< All pages
    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16> m_shortFixedSizeArrayRecycler; ///< For recycling voxel data
    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8> m_byteFixedSizeArrayRecycler; ///< For recycling voxel data
};

#endif // ChunkAllocator_h__
