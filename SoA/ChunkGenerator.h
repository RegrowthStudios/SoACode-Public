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

class ChunkQuery {
public:
    /// Blocks current thread until consumption is performed
    void block() {
        std::unique_lock<std::mutex> lck(m_lock);
        if (m_chunk) return;
        m_cond.wait(lck);
    }

    // Returns nullptr if not completed
    NChunk* getChunk() const { return m_chunk; }

    i32v3 chunkPos;
private:
    NChunk* m_chunk = nullptr;
    std::mutex m_lock;
    std::condition_variable m_cond;
};

class ChunkGenerator {
public:
    void init(ChunkAllocator* chunkAllocator, vcore::ThreadPool<WorkerData>* threadPool, PlanetGenData* genData);
    void generateChunk(ChunkQuery* query);
private:
    ProceduralChunkGenerator proceduralGenerator;
    vcore::ThreadPool<WorkerData>* m_threadPool = nullptr;
    ChunkAllocator* m_allocator = nullptr;
};

#endif // ChunkGenerator_h__
