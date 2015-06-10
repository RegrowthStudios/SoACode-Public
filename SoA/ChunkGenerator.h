///
/// ChunkGenerator.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 10 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef ChunkGenerator_h__
#define ChunkGenerator_h__

#include "VoxPool.h"
#include "ProceduralChunkGenerator.h"
#include "PlanetData.h"
#include <Vorb/ThreadPool.h>

class ChunkAllocator;

enum ChunkGenLevel { NONE = 0, TERRAIN, FLORA, LIGHT };

class ChunkQuery {
    friend class GenerateTask;
    friend class NChunkGrid;
public:
    /// Blocks current thread until consumption is performed
    void block() {
        std::unique_lock<std::mutex> lck(m_lock);
        if (m_chunk) return;
        m_cond.wait(lck);
    }

    const bool& isFinished() const { return m_isFinished; }
    NChunk* getChunk() const { return m_chunk; }

    i32v3 chunkPos;
    ChunkGenLevel genLevel;
private:
    bool m_isFinished = false;
    NChunk* m_chunk = nullptr;
    std::mutex m_lock;
    std::condition_variable m_cond;
};

class ChunkGenerator {
    friend class GenerateTask;
public:
    void init(ChunkAllocator* chunkAllocator,
              vcore::ThreadPool<WorkerData>* threadPool,
              PlanetGenData* genData);
    void submitQuery(ChunkQuery* query);
    void onQueryFinish(ChunkQuery* query);
    // Updates finished queries
    void update();
private:
    moodycamel::ConcurrentQueue<ChunkQuery*> m_finishedQueries;

    ProceduralChunkGenerator m_proceduralGenerator;
    vcore::ThreadPool<WorkerData>* m_threadPool = nullptr;
    ChunkAllocator* m_allocator = nullptr;
};

#endif // ChunkGenerator_h__
