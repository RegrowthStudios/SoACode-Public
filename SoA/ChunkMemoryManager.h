///
/// ChunkMemoryManager.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 26 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Handles memory management for chunks
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

    /// Reserves internal memory
    /// Calling this will invalidate all chunks!
    void setSize(size_t numChunks);
    /// Gets a new chunk ID
    Chunk* getNewChunk();
    /// Frees a chunk
    void freeChunk(Chunk* chunk);
    /// Gets a chunk from an ID
    /// The reference will become invalid
    /// if getNewChunk() needs to allocate new
    /// memory.
    inline Chunk& getChunk(ChunkID chunkID) {
        return m_chunkMemory[chunkID];
    }
    const size_t& getMaxSize() const { return m_maxSize; }

    const std::vector<Chunk*>& getActiveChunks() const { return m_activeChunks; }

    const Chunk* getChunkMemory(OUT size_t& size) const { size = m_maxSize; return m_chunkMemory; }
private:
    std::vector<Chunk*> m_activeChunks;
    Chunk* m_chunkMemory = nullptr; ///< All chunks
    size_t m_maxSize = 0;
    std::vector<ChunkID> m_freeChunks; ///< List of free chunks
    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16> m_shortFixedSizeArrayRecycler; ///< For recycling voxel data
    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8> m_byteFixedSizeArrayRecycler; ///< For recycling voxel data
};

#endif // ChunkMemoryManager_h__
