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
#include "concurrentqueue.h"
#include "NChunk.h"

#include <memory>
#include <map>
#include <vector>
#include <Vorb/utils.h>

class ChunkAllocator;

class NChunkGrid {
public:
    void init(WorldCubeFace face, ChunkAllocator* chunkAllocator, 
              OPT vcore::ThreadPool<WorkerData>* threadPool,
              ui32 generatorsPerRow,
              PlanetGenData* genData);

    void addChunk(NChunk* chunk);

    void removeChunk(NChunk* chunk);

    NChunk* getChunk(const f64v3& voxelPos);

    NChunk* getChunk(const i32v3& chunkPos);

    const NChunk* getChunk(const i32v3& chunkPos) const;

    // Will generate chunk if it doesn't exist
    void submitQuery(ChunkQuery* query);

    /// Gets a chunkGridData for a specific 2D position
    /// @param gridPos: The grid position for the data
    std::shared_ptr<NChunkGridData> getChunkGridData(const i32v2& gridPos) const;

    // Processes chunk queries
    void update();

    NChunk* getActiveChunks() const { return m_activeChunks; }
    const ui32& getNumActiveChunks() const { return m_numActiveChunks; }

private:
    void connectNeighbors(NChunk* chunk);
    void disconnectNeighbors(NChunk* chunk);

    moodycamel::ConcurrentQueue<ChunkQuery*> m_queries;

    ChunkAllocator* m_allocator = nullptr;
    ChunkGenerator* m_generators = nullptr;

    NChunk* m_activeChunks = nullptr; ///< Linked list of chunks
    ui32 m_numActiveChunks = 0;

    std::unordered_map<i32v3, NChunk*> m_chunkMap; ///< hashmap of chunks
    std::unordered_map<i32v2, std::shared_ptr<NChunkGridData> > m_chunkGridDataMap; ///< 2D grid specific data
    
    ui32 m_generatorsPerRow;
    ui32 m_numGenerators;
    WorldCubeFace m_face = FACE_NONE;

    static volatile ChunkID m_nextAvailableID;
};

#endif // ChunkGrid_h__
