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
    ChunkHandle() : 
        m_acquired(false),
        m_chunk(nullptr),
        m_id({}) {
        // Empty
    }
    ChunkHandle(const ChunkHandle& other);
    ChunkHandle& operator= (const ChunkHandle& other);
    ChunkHandle(ChunkHandle&& other) :
        m_acquired(other.m_acquired),
        m_chunk(other.m_chunk),
        m_id(other.m_id) {

        other.m_acquired = false;
        other.m_chunk = nullptr;
        other.m_id = 0;
    }
    ChunkHandle& operator= (ChunkHandle&& other) {
        m_acquired = other.m_acquired;
        m_chunk = other.m_chunk;
        m_id = other.m_id;

        other.m_acquired = false;
        other.m_chunk = nullptr;
        other.m_id.id = 0;

        return *this;
    }

    void acquireSelf();
    ChunkHandle acquire();
    void release();
    bool isAquired() const { return m_acquired; }

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
    union {
        Chunk* m_chunk;
        ChunkAccessor* m_accessor;
    };
    ChunkID m_id;
    bool m_acquired;
};

#endif // ChunkHandle_h__
