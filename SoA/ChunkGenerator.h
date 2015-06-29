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
#include "GenerateTask.h"
#include <Vorb/ThreadPool.h>

class ChunkAllocator;
class ChunkGridData;

enum ChunkGenLevel { GEN_NONE = 0, GEN_TERRAIN, GEN_FLORA, GEN_SCRIPT, GEN_DONE };

class ChunkQuery {
    friend class ChunkGenerator;
    friend class GenerateTask;
    friend class ChunkGrid;
public:
    void set(const i32v3& chunkPos, ChunkGenLevel genLevel, bool shouldDelete) {
        this->chunkPos = chunkPos;
        this->genLevel = genLevel;
        this->shouldDelete = shouldDelete;
    }

    /// Blocks current thread until the query is finished
    void block() {
        std::unique_lock<std::mutex> lck(m_lock);
        if (m_chunk) return;
        m_cond.wait(lck);
    }

    const bool& isFinished() const { return m_isFinished; }
    Chunk* getChunk() const { return m_chunk; }

    i32v3 chunkPos;
    ChunkGenLevel genLevel;
    GenerateTask genTask; ///< For if the query results in generation
    bool shouldDelete = false;
private:
    bool m_isFinished = false;
    Chunk* m_chunk = nullptr;
    std::mutex m_lock;
    std::condition_variable m_cond;
};

// Data stored in Chunk and used only by ChunkGenerator
struct ChunkGenQueryData {
    friend class ChunkGenerator;
    friend class Chunk;
private:
    ChunkQuery* current = nullptr;
    std::vector<ChunkQuery*> pending;
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
    std::map < ChunkGridData*, std::vector<ChunkQuery*> >m_pendingQueries; ///< Queries waiting on height map

    ProceduralChunkGenerator m_proceduralGenerator;
    vcore::ThreadPool<WorkerData>* m_threadPool = nullptr;
    ChunkAllocator* m_allocator = nullptr;
};

#endif // ChunkGenerator_h__
