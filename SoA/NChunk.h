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
#include "MetaSection.h"
#include <Vorb/FixedSizeArrayRecycler.hpp>

class NChunk;
typedef NChunk* NChunkPtr;

class NChunkGridData {
public:
    NChunkGridData(const ChunkPosition2D& pos) {
        gridPosition = pos;
    }
    NChunkGridData(const ChunkPosition3D& pos) {
        gridPosition.pos = i32v2(pos.pos.x, pos.pos.z);
        gridPosition.face = pos.face;
    }

    ChunkPosition2D gridPosition;
    PlanetHeightData heightData[CHUNK_LAYER];
    volatile bool wasRequestSent = false; /// True when heightmap was already sent for gen
    volatile bool isLoaded = false;
    int refCount = 1;
};

class NChunk {
public:
    void init(const ChunkPosition3D& pos);
    void set(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler,
             vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8>* byteRecycler);

    const ChunkPosition3D& getPosition() const { return m_position; }

    NChunkGridData* gridData = nullptr;
    MetaFieldInformation meta;
    NChunkPtr neighbors[6];
private:
    ChunkPosition3D m_position;
    // TODO(Ben): Think about data locality.
    vvox::SmartVoxelContainer<ui16> m_blocks;
    vvox::SmartVoxelContainer<ui8> m_sunlight;
    vvox::SmartVoxelContainer<ui16> m_lamp;
    vvox::SmartVoxelContainer<ui16> m_tertiary;
};

#endif // NChunk_h__
