///
/// ChunkGrid.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 26 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Grid of chunks for a voxel world
///

#pragma once

#ifndef ChunkGrid_h__
#define ChunkGrid_h__

class ChunkMemoryManager;
class GeneratedTreeNodes;

#include "Chunk.h"

class ChunkGrid {
public:
    ChunkGrid(ChunkMemoryManager* chunkMemoryManager);

    void addChunk(Chunk* chunk);
    void addChunk(ChunkID chunkID);

    void removeChunk(Chunk* chunk);
    void removeChunk(ChunkID chunkID);

    /// Gets a chunk at a given floating point position
    /// @param position: The position
    /// Returns nullptr if no chunk is found, otherwise returns the chunk
    Chunk* getChunk(const f64v3& position);

    /// Gets a chunk at a given chunk grid position
    /// @param chunkPos: The chunk grid position, units are of chunk length
    /// Returns nullptr if no chunk is found, otherwise returns the chunk
    Chunk* getChunk(const i32v3& chunkPos);

    /// Gets a const chunk at a given chunk grid position
    /// @param chunkPos: The chunk grid position, units are of chunk length
    /// Returns nullptr if no chunk is found, otherwise returns the chunk
    const Chunk* getChunk(const i32v3& chunkPos) const;
private:
    std::vector<ChunkID> m_chunks; // TODO(Ben): Is this needed here?
    /// hashmap of chunks
    std::unordered_map<i32v3, ChunkID> m_chunkMap;
    /// Indexed by (x,z)
    std::unordered_map<i32v2, std::shared_ptr<ChunkGridData> > m_chunkGridDataMap;

    std::vector<GeneratedTreeNodes*> m_treesToPlace; ///< List of tree batches we need to place

    ChunkMemoryManager* m_chunkMemoryManager = nullptr;

};

#endif // ChunkGrid_h__
