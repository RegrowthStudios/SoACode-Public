//
// ChunkQuery.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 1 Aug 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// Class for querying for a chunk with a certain generation level
//

#pragma once

#ifndef ChunkQuery_h__
#define ChunkQuery_h__

#include "ChunkHandle.h"
#include "GenerateTask.h"

#include <Vorb/PtrRecycler.hpp>

enum ChunkGenLevel { GEN_NONE = 0, GEN_TERRAIN, GEN_FLORA, GEN_SCRIPT, GEN_DONE };

class ChunkGrid;

class ChunkQuery {
    friend class ChunkGenerator;
    friend class GenerateTask;
    friend class ChunkGrid;
public:
    void release();

    /// Blocks current thread until the query is finished
    void block() {
        std::unique_lock<std::mutex> lck(m_lock);
        if (isFinished()) return;
        m_cond.wait(lck);
    }

    const bool& isFinished() const { return m_isFinished; }

    i32v3 chunkPos;
    ChunkGenLevel genLevel;
    GenerateTask genTask; ///< For if the query results in generation
    ChunkHandle chunk; ///< Gets set on submitQuery
    bool shouldRelease;
    ChunkGrid* grid;
private:
    bool m_isFinished;
    std::mutex m_lock;
    std::condition_variable m_cond;
};

#endif // ChunkQuery_h__
