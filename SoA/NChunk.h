//
// NChunk.h
// Seed of Andromeda
//
// Created by Cristian Zaloj on 25 May 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// 
//

#pragma once

#ifndef NChunk_h__
#define NChunk_h__

#include "Constants.h"
#include "SmartVoxelContainer.hpp"
#include "VoxelCoordinateSpaces.h"
#include "PlanetHeightData.h"
#include <Vorb/FixedSizeArrayRecycler.hpp>

class NChunk;
typedef NChunk* NChunkPtr;

class NChunkGridData {
public:
    NChunkGridData(const i32v2& pos, WorldCubeFace face) {
        gridPosition.pos = pos;
        gridPosition.face = face;
    }

    ChunkPosition2D gridPosition;
    int refCount = 1;
    PlanetHeightData heightData[CHUNK_LAYER];
    volatile bool wasRequestSent = false; /// True when heightmap was already sent for gen
    volatile bool isLoaded = false;
};

class NChunk {
public:
    void init(const ChunkPosition3D& pos);
    void set(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler,
             vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8>* byteRecycler);

    NChunkPtr neighbors[6];
private:
    vvox::SmartVoxelContainer<ui16> m_blocks;
    vvox::SmartVoxelContainer<ui8> m_sunlight;
    vvox::SmartVoxelContainer<ui16> m_lamp;
    vvox::SmartVoxelContainer<ui16> m_tertiary;
};

#endif // NChunk_h__
