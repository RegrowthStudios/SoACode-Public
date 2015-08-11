//
// ChunkHandle.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 30 Jul 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// Does ref counting.
//

#pragma once

#ifndef ChunkHandle_h__

#include "ChunkID.h"

class Chunk;

class ChunkHandle {
    friend class ChunkAccessor;
public:
    ChunkHandle acquire();
    void release();
    bool isAquired() const { return m_chunk != nullptr; }

    operator Chunk&() {
        return *m_chunk;
    }
    operator const Chunk&() const {
        return *m_chunk;
    }

    operator Chunk*() {
        return m_chunk;
    }
    operator const Chunk*() const {
        return m_chunk;
    }

    Chunk* operator->() {
        return m_chunk;
    }
    const Chunk* operator->() const {
        return m_chunk;
    }

    const ChunkID& getID() const {
        return m_id;
    }
private:
    Chunk* m_chunk = nullptr;
    ChunkID m_id;
};

#endif // ChunkHandle_h__
