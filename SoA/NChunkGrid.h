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

#include "VoxelCoordinateSpaces.h"
#include <memory>
#include <map>
#include <vector>

class ChunkMemoryManager;
class GeneratedTreeNodes;

class NChunk;
struct NChunkGridData;

class NChunkGrid {
public:
    friend class SphericalVoxelComponentUpdater;

    void init(WorldCubeFace face, ui32 generatorsPerRow);

    /// Adds a chunk to the grid
    /// @param chunk: The chunk to add
    void addChunk(NChunk* chunk);

    /// Removes a chunk from the grid
    /// @param chunk: The chunk to remove
    void removeChunk(NChunk* chunk);

    /// Gets a chunk at a given floating point position
    /// @param position: The position
    /// Returns nullptr if no chunk is found, otherwise returns the chunk
    NChunk* getChunk(const f64v3& position);

    /// Gets a chunk at a given chunk grid position
    /// @param chunkPos: The chunk grid position, units are of chunk length
    /// Returns nullptr if no chunk is found, otherwise returns the chunk
    NChunk* getChunk(const i32v3& chunkPos);

    /// Gets a const chunk at a given chunk grid position
    /// @param chunkPos: The chunk grid position, units are of chunk length
    /// Returns nullptr if no chunk is found, otherwise returns the chunk
    const NChunk* getChunk(const i32v3& chunkPos) const;

    /// Gets a chunkGridData for a specific 2D position
    /// @param gridPos: The grid position for the data
    NChunkGridData* getChunkGridData(const i32v2& gridPos) {
        auto it = m_chunkGridDataMap.find(gridPos);
        if (it == m_chunkGridDataMap.end()) return nullptr;
        return it->second;
    }

    /// Gets the world cube face for this grid
    WorldCubeFace getFace() const { return m_face; }

private:
    std::vector<ChunkGenerator> m_generators;
    std::unordered_map<i32v3, NChunk*> m_chunkMap; ///< hashmap of chunks
    std::unordered_map<i32v2, NChunkGridData*> m_chunkGridDataMap; ///< 2D grid specific data

    WorldCubeFace m_face = FACE_NONE;
};

#endif // ChunkGrid_h__
