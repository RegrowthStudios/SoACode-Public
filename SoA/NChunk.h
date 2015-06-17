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
    NChunkGridData(const ChunkPosition3D& pos) {
        gridPosition.pos = i32v2(pos.pos.x, pos.pos.z);
        gridPosition.face = pos.face;
    }

    ChunkPosition2D gridPosition;
    PlanetHeightData heightData[CHUNK_LAYER];
    bool isLoading = false;
    bool isLoaded = false;
    int refCount = 1;
};

class NChunk {
    friend class ChunkGenerator;
    friend class ProceduralChunkGenerator;
    friend class PagedChunkAllocator;
    friend class SphericalVoxelComponentUpdater;
    friend class NChunkGrid;
    friend class RenderTask;
public:
    void init(const ChunkPosition3D& pos);
    void setRecyclers(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler,
                      vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8>* byteRecycler);
    void updateContainers();

    /************************************************************************/
    /* Getters                                                              */
    /************************************************************************/
    const ChunkPosition3D& getChunkPosition() const { return m_chunkPosition; }
    const VoxelPosition3D& getVoxelPosition() const { return m_voxelPosition; }
    bool hasAllNeighbors() const { return m_numNeighbors == 6u; }
    const bool& isInRange() const { return m_isInRange; }
    const f32& getDistance2() const { return m_distance2; }
    NChunkPtr getNextActive() const { return m_nextActive; }
    NChunkPtr getPrevActive() const { return m_prevActive; }

    inline ui16 getBlockData(int c) const {
        return m_blocks[c];
    }
    inline ui16 getLampLight(int c) const {
        return m_lamp[c];
    }
    inline ui8 getSunlight(int c) const {
        return m_sunlight[c];
    }
    inline ui16 getTertiaryData(int c) const {
        return m_tertiary[c];
    }

    // True when the chunk needs to be meshed
    bool needsRemesh() { return m_remeshFlags != 0; }
    // Marks the chunks as dirty and flags for a re-mesh
    void flagDirty() { m_isDirty = true; m_remeshFlags |= 1; }

    // TODO(Ben): This can be better
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }

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
    int refCount = 0;
    ChunkGenLevel genLevel = ChunkGenLevel::GEN_NONE;
    volatile bool isAccessible = false;
    volatile bool queuedForMesh = false;
private:
    // For generation
    ChunkGenQueryData m_genQueryData;
    // For ChunkGrid
    NChunkPtr m_nextActive = nullptr;
    NChunkPtr m_prevActive = nullptr;

    ui32 m_numNeighbors = 0u;
    ui32 m_loadingNeighbors = 0u; ///< Seems like a good idea to get rid of isAccesible
    ChunkPosition3D m_chunkPosition;
    VoxelPosition3D m_voxelPosition;
    // TODO(Ben): Think about data locality.
    vvox::SmartVoxelContainer<ui16> m_blocks;
    vvox::SmartVoxelContainer<ui8> m_sunlight;
    vvox::SmartVoxelContainer<ui16> m_lamp;
    vvox::SmartVoxelContainer<ui16> m_tertiary;
    ui8 m_remeshFlags;
    bool m_isInRange;
    bool m_isDirty;
    f32 m_distance2; //< Squared distance
};

#endif // NChunk_h__
