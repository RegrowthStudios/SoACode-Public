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
        if (isFinished()) return;
        m_cond.wait(lck);
    }

    const bool& isFinished() const { return m_isFinished; }

    i32v3 chunkPos;
    ChunkGenLevel genLevel;
    GenerateTask genTask; ///< For if the query results in generation
    ChunkHandle chunk; ///< Gets set on submitQuery
    bool shouldDelete = false;
private:
    bool m_isFinished = false;
    std::mutex m_lock;
    std::condition_variable m_cond;
};

#endif // ChunkQuery_h__
