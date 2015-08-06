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

#include <concurrentqueue.h>
#include <Vorb/utils.h>
#include <Vorb/IDGenerator.h>

#include "VoxelCoordinateSpaces.h"
#include "ChunkGenerator.h"
#include "Chunk.h"
#include "ChunkAllocator.h"
#include "ChunkAccessor.h"
#include "ChunkHandle.h"

class ChunkGrid {
public:
    void init(WorldCubeFace face,
              OPT vcore::ThreadPool<WorkerData>* threadPool,
              ui32 generatorsPerRow,
              PlanetGenData* genData,
              PagedChunkAllocator* allocator);
    void dispose();

    ChunkHandle getChunk(const f64v3& voxelPos);
    ChunkHandle getChunk(const i32v3& chunkPos);

    // Will generate chunk if it doesn't exist
    void submitQuery(ChunkQuery* query);

    /// Gets a chunkGridData for a specific 2D position
    /// @param gridPos: The grid position for the data
    ChunkGridData* getChunkGridData(const i32v2& gridPos);

    // Processes chunk queries
    void update();

    void acquireNeighbors(ChunkHandle chunk);
    void releaseNeighbors(ChunkHandle chunk);

    const std::vector<ChunkHandle>& getActiveChunks() const { return m_activeChunks; }

private:
    /************************************************************************/
    /* Event Handlers                                                       */
    /************************************************************************/
    void onAccessorAdd(Sender s, ChunkHandle chunk);
    void onAccessorRemove(Sender s, ChunkHandle chunk);

    moodycamel::ConcurrentQueue<ChunkQuery*> m_queries;

    ChunkAccessor m_accessor;
    ChunkGenerator* m_generators = nullptr;

    std::mutex m_lckActiveChunks;
    std::vector<ChunkHandle> m_activeChunks;

    // TODO(Ben): Compare to std::map performance
    std::mutex m_lckGridData;
    std::unordered_map<i32v2, ChunkGridData*> m_chunkGridDataMap; ///< 2D grid specific data
    
    vcore::IDGenerator<ChunkID> m_idGenerator;

    ui32 m_generatorsPerRow;
    ui32 m_numGenerators;
    WorldCubeFace m_face = FACE_NONE;
};

#endif // ChunkGrid_h__
