//
// ChunkID.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 3 Aug 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// 64 bit ID type for Chunks.
//

#pragma once

#ifndef ChunkID_h__
#define ChunkID_h__

#include "Vorb/types.h"

// Chunk ID, derived from position.
struct ChunkID {
    ChunkID() : id(0) {};
    ChunkID(i32 x, i32 y, i32 z) : x(x), y(y), z(z) {};
    ChunkID(const i32v3& p) : x(p.x), y(p.y), z(p.z) {};
    ChunkID(ui64 id) : id(id) {};
    union {
        struct {
            i64 x : 24;
            i64 y : 16;
            i64 z : 24;
        };
        ui64 id;
    };
    operator ui64() { return id; }
    operator const ui64() const { return id; }
};
// Hash for ID
template <>
struct std::hash<ChunkID> {
    size_t operator()(const ChunkID& id) const {
        std::hash<ui64> h;
        return h(id.id);
    }
};

static_assert(sizeof(ChunkID) == 8, "ChunkID is not 64 bits");

#endif // ChunkID_h__
