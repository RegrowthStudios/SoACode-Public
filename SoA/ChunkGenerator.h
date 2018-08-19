///
/// ChunkGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jun 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// 
///

#pragma once

#ifndef ChunkGenerator_h__
#define ChunkGenerator_h__

#include <Vorb/ThreadPool.h>

#include "VoxPool.h"
#include "ProceduralChunkGenerator.h"
#include "PlanetGenData.h"
#include "GenerateTask.h"
#include "ChunkQuery.h"

class PagedChunkAllocator;
class ChunkGridData;
class ChunkGrid;

// Data stored in Chunk and used only by ChunkGenerator
struct ChunkGenQueryData {
    friend class ChunkGenerator;
    friend class ChunkGrid;
    friend class PagedChunkAllocator;
private:
    ChunkQuery* current = nullptr;
    std::vector<ChunkQuery*> pending;
};

class ChunkGenerator {
    friend class GenerateTask;
public:
    void init(vcore::ThreadPool<WorkerData>* threadPool,
              PlanetGenData* genData,
              ChunkGrid* grid);
    void submitQuery(ChunkQuery* query);
    void finishQuery(ChunkQuery* query);
    // Updates finished queries
    void update();

    Event<ChunkHandle&, ChunkGenLevel> onGenFinish;
private:
    void tryFlagMeshableNeighbors(ChunkHandle& ch);
    void flagMeshbleNeighbor(ChunkHandle& n, ui32 bit);

    moodycamel::ConcurrentQueue<ChunkQuery*> m_finishedQueries;
    std::map < ChunkGridData*, std::vector<ChunkQuery*> >m_pendingQueries; ///< Queries waiting on height map

    ChunkGrid* m_grid = nullptr;
    ProceduralChunkGenerator m_proceduralGenerator;
    vcore::ThreadPool<WorkerData>* m_threadPool = nullptr;
};

#endif // ChunkGenerator_h__
