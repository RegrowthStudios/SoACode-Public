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
    friend class ChunkMeshManager;
public:
    void init(WorldCubeFace face,
              OPT vcore::ThreadPool<WorkerData>* threadPool,
              ui32 generatorsPerRow,
              PlanetGenData* genData,
              PagedChunkAllocator* allocator);
    void dispose();

    /// Will generate chunk if it doesn't exist
    /// @param gridPos: The position of the chunk to get.
    /// @param genLevel: The required generation level.
    /// @param shouldRelease: Will automatically release when true.
    ChunkQuery* submitQuery(const i32v3& chunkPos, ChunkGenLevel genLevel, bool shouldRelease);
    /// Releases and recycles a query.
    void releaseQuery(ChunkQuery* query);

    /// Gets a chunkGridData for a specific 2D position
    /// @param gridPos: The grid position for the data
    ChunkGridData* getChunkGridData(const i32v2& gridPos);

    // Processes chunk queries and set active chunks
    void update();

    const std::vector<ChunkHandle>& getActiveChunks() const { return m_activeChunks; }

    ChunkGenerator* generators = nullptr;
    ui32 generatorsPerRow;
    ui32 numGenerators;

    ChunkAccessor accessor;
private:
    /************************************************************************/
    /* Event Handlers                                                       */
    /************************************************************************/
    void onAccessorAdd(Sender s, ChunkHandle chunk);
    void onAccessorRemove(Sender s, ChunkHandle chunk);

    moodycamel::ConcurrentQueue<ChunkQuery*> m_queries;

    

    std::vector<ChunkHandle> m_activeChunks;

    // To prevent needing lock on m_activeChunks;
    std::mutex m_lckAddOrRemove;
    std::vector<std::pair<ChunkHandle, bool /*true = add*/>> m_activeChunksToAddOrRemove;

    // TODO(Ben): Compare to std::map performance
    std::mutex m_lckGridData;
    std::unordered_map<i32v2, ChunkGridData*> m_chunkGridDataMap; ///< 2D grid specific data
    
    vcore::IDGenerator<ChunkID> m_idGenerator;

    std::mutex m_lckQueryRecycler;
    PtrRecycler<ChunkQuery> m_queryRecycler;


    WorldCubeFace m_face = FACE_NONE;
};

#endif // ChunkGrid_h__
