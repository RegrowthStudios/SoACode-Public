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

// Chunk ID, derived from position.
struct ChunkID {
    ChunkID() : id(0) {};
    ChunkID(i32 x, i32 y, i32 z) : x(x), y(y), z(z) {};
    ChunkID(ui64 id) : id(id) {};
    union {
        struct {
            i32 x : 24;
            i32 y : 16;
            i32 z : 24;
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

#endif // ChunkID_h__
