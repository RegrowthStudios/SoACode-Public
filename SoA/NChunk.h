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
#include "ChunkGenerator.h"
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
    volatile bool isLoading = false;
    volatile bool isLoadingFinished = false;
    int refCount = 1;
};

class NChunk {
    friend class ChunkGenerator;
    friend class ProceduralChunkGenerator;
    friend class PagedChunkAllocator;
    friend class SphericalVoxelComponentUpdater;
public:
    void init(const ChunkPosition3D& pos);
    void setRecyclers(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler,
                      vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8>* byteRecycler);

    /************************************************************************/
    /* Getters                                                              */
    /************************************************************************/
    const ChunkPosition3D& getPosition() const { return m_position; }
    bool hasAllNeighbors() const { return m_numNeighbors == 6u; }
    const bool& isInRange() const { return m_isInRange; }
    const f32& getDistance2() const { return m_distance2; }

    /************************************************************************/
    /* Members                                                              */
    /************************************************************************/
    NChunkGridData* gridData = nullptr;
    MetaFieldInformation meta;    
    union {
        struct {
            NChunkPtr left, right, bottom, top, back, front;
        };
        NChunkPtr neighbors[6];
    };
    std::mutex mutex;
    volatile int refCount = 0;
    ChunkGenLevel genLevel = ChunkGenLevel::NONE;
private:
    // For generation
    ChunkQuery* m_currentQuery = nullptr;
    std::vector<ChunkQuery*> m_pendingQueries;

    ui32 m_numNeighbors = 0u;
    ChunkPosition3D m_position;
    // TODO(Ben): Think about data locality.
    vvox::SmartVoxelContainer<ui16> m_blocks;
    vvox::SmartVoxelContainer<ui8> m_sunlight;
    vvox::SmartVoxelContainer<ui16> m_lamp;
    vvox::SmartVoxelContainer<ui16> m_tertiary;
    bool m_isInRange;
    f32 m_distance2; //< Squared distance
};

#endif // NChunk_h__
