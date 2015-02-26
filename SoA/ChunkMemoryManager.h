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
    // Reserves internal memory
    void reserve(int numChunks);
    // Gets a new chunk ID
    ChunkID getNewChunk();
    // Frees a chunk
    void freeChunk(ChunkID chunkID);
    // Gets a chunk from an ID
    // The reference will become invalid
    // if getNewChunk() needs to allocate new
    // memory.
    Chunk& getChunk(ChunkID chunkID);
private:
    std::vector<Chunk> m_chunkMemory; ///< All chunks
    std::vector<ChunkID> m_freeChunks; ///< List of free chunks
    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16> m_shortFixedSizeArrayRecycler; ///< For recycling voxel data
    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8> m_byteFixedSizeArrayRecycler; ///< For recycling voxel data
};

#endif // ChunkMemoryManager_h__
