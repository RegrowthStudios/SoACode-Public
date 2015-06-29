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

class Chunk;
typedef Chunk* ChunkPtr;
typedef ui32 ChunkID;

class ChunkGridData {
public:
    ChunkGridData(const ChunkPosition3D& pos) {
        gridPosition.pos = i32v2(pos.pos.x, pos.pos.z);
        gridPosition.face = pos.face;
    }

    ChunkPosition2D gridPosition;
    PlanetHeightData heightData[CHUNK_LAYER];
    bool isLoading = false;
    bool isLoaded = false;
    int refCount = 1;
};

class Chunk {
    friend class ChunkGenerator;
    friend class ProceduralChunkGenerator;
    friend class PagedChunkAllocator;
    friend class SphericalVoxelComponentUpdater;
    friend class ChunkGrid;
    friend class RenderTask;
public:
    void init(ChunkID id, const ChunkPosition3D& pos);
    void setRecyclers(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler);
    void updateContainers();

    /************************************************************************/
    /* Getters                                                              */
    /************************************************************************/
    const ChunkPosition3D& getChunkPosition() const { return m_chunkPosition; }
    const VoxelPosition3D& getVoxelPosition() const { return m_voxelPosition; }
    bool hasAllNeighbors() const { return m_numNeighbors == 6u; }
    const bool& isInRange() const { return m_isInRange; }
    const f32& getDistance2() const { return m_distance2; }
    const ChunkID& getID() const { return m_id; }

    inline ui16 getBlockData(int c) const {
        return m_blocks.get(c);
    }
    inline ui16 getTertiaryData(int c) const {
        return m_tertiary.get(c);
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
    std::shared_ptr<ChunkGridData> gridData = nullptr;
    MetaFieldInformation meta;    
    union {
        struct {
            ChunkPtr left, right, bottom, top, back, front;
        };
        ChunkPtr neighbors[6];
    };
    std::mutex mutex;
    int refCount;
    int numBlocks;
    ChunkGenLevel genLevel = ChunkGenLevel::GEN_NONE;
    volatile bool isAccessible = false;
    volatile bool queuedForMesh = false;
    bool hasCreatedMesh = false;

    static ui32 vboIndicesID;
private:
    // For generation
    ChunkGenQueryData m_genQueryData;

    ui32 m_numNeighbors = 0u;
    ui32 m_loadingNeighbors = 0u; ///< Seems like a good idea to get rid of isAccesible
    ChunkPosition3D m_chunkPosition;
    VoxelPosition3D m_voxelPosition;
    // TODO(Ben): Think about data locality.
    vvox::SmartVoxelContainer<ui16> m_blocks;
    vvox::SmartVoxelContainer<ui16> m_tertiary;
    ui8 m_remeshFlags;
    bool m_isInRange;
    bool m_isDirty;
    f32 m_distance2; //< Squared distance
    ChunkID m_id;
};

#endif // NChunk_h__
