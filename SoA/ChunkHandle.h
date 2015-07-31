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

class ChunkHandle {
    friend class ChunkAccessor;
public:
    ChunkHandle acquire();
    void release();

    operator Chunk&() {
        return *m_chunk;
    }
    operator const Chunk&() const {
        return *m_chunk;
    }

    Chunk* operator->() {
        return m_chunk;
    }
    const Chunk* operator->() const {
        return m_chunk;
    }
private:
    Chunk* m_chunk;
};

#endif // ChunkHandle_h__
