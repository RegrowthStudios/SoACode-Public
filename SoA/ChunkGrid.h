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
    friend class SphericalVoxelComponentUpdater;

    ~ChunkGrid();

    void addChunk(Chunk* chunk);

    void removeChunk(Chunk* chunk);

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

    std::shared_ptr<ChunkGridData> getChunkGridData(const i32v2& gridPos) {
        auto it = m_chunkGridDataMap.find(gridPos);
        if (it == m_chunkGridDataMap.end()) return nullptr;
        return it->second;
    }

    std::vector<GeneratedTreeNodes*> treesToPlace; ///< List of tree batches we need to place
private:
    std::vector<Chunk*> m_chunks; // TODO(Ben): Is this needed here?
    /// hashmap of chunks
    std::unordered_map<i32v3, Chunk*> m_chunkMap;
    /// Indexed by (x,z)
    std::unordered_map<i32v2, std::shared_ptr<ChunkGridData> > m_chunkGridDataMap;
};

#endif // ChunkGrid_h__
