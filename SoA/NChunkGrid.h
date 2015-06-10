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
#include "ChunkGenerator.h"
#include <memory>
#include <map>
#include <vector>

class NChunk;
class ChunkAllocator;
struct NChunkGridData;

class NChunkGrid {
public:
    void init(WorldCubeFace face, ChunkAllocator* chunkAllocator, 
              OPT vcore::ThreadPool<WorkerData>* threadPool, ui32 generatorsPerRow);

    void addChunk(NChunk* chunk);

    void removeChunk(NChunk* chunk);

    NChunk* getChunk(const f64v3& voxelPos);

    NChunk* getChunk(const i32v3& chunkPos);

    const NChunk* getChunk(const i32v3& chunkPos) const;

    // Will generate chunk if it doesn't exist
    void submitQuery(ChunkQuery* query) const;

    /// Gets a chunkGridData for a specific 2D position
    /// @param gridPos: The grid position for the data
    NChunkGridData* getChunkGridData(const i32v2& gridPos) const;

    // Processes chunk queries
    void update();

private:
    moodycamel::ConcurrentQueue<ChunkQuery*> m_queries;
    ChunkAllocator* m_allocator = nullptr;
    std::vector<ChunkGenerator> m_generators;

    std::unordered_map<i32v3, NChunk*> m_chunkMap; ///< hashmap of chunks
    std::unordered_map<i32v2, NChunkGridData*> m_chunkGridDataMap; ///< 2D grid specific data
    
    ui32 m_generatorsPerRow;
    ui32 m_numGenerators;
    WorldCubeFace m_face = FACE_NONE;
};

#endif // ChunkGrid_h__
