///
/// ChunkMemoryManager.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 26 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Paged memory management system for chunks
///

#pragma once

#ifndef ChunkMemoryManager_h__
#define ChunkMemoryManager_h__

#include "Chunk.h"

class ChunkMemoryManager {
public:
    friend class SphericalVoxelComponentUpdater;

    ChunkMemoryManager();
    ~ChunkMemoryManager();

    /// Gets a new chunk ID
    Chunk* getNewChunk();
    /// Frees a chunk
    void freeChunk(Chunk* chunk);

    const std::vector<Chunk*>& getActiveChunks() const { return m_activeChunks; }
private:
    static const size_t CHUNK_PAGE_SIZE = 2048;
    struct ChunkPage {
        Chunk chunks[CHUNK_PAGE_SIZE];
    };

    // TODO(BEN): limit number of pages that can be allocated
    std::vector<Chunk*> m_activeChunks; ///< List of currently active chunks
    std::vector<Chunk*> m_freeChunks; ///< List of inactive chunks
    std::vector<ChunkPage*> m_chunkPages; ///< All pages
    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16> m_shortFixedSizeArrayRecycler; ///< For recycling voxel data
    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8> m_byteFixedSizeArrayRecycler; ///< For recycling voxel data
};

#endif // ChunkMemoryManager_h__
