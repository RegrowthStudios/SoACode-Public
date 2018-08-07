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
#include "ChunkID.h"
#include <Vorb/FixedSizeArrayRecycler.hpp>

#if defined(_MSC_VER)
#define ALIGNED_(x) __declspec(align(x))
#else
#define ALIGNED_(x) __attribute__ ((aligned(x)))
#endif

class Chunk;
typedef Chunk* ChunkPtr;

// TODO(Ben): Move to file
typedef ui16 BlockIndex;

class ChunkGridData {
public:
    ChunkGridData():isLoading(false), isLoaded(false), refCount(1){};
    ChunkGridData(const ChunkPosition3D& pos):isLoading(false), isLoaded(false), refCount(1)
    {
        gridPosition.pos = i32v2(pos.pos.x, pos.pos.z);
        gridPosition.face = pos.face;
    }

    ChunkPosition2D gridPosition;
    PlanetHeightData heightData[CHUNK_LAYER];
    bool isLoading;
    bool isLoaded;
    int refCount;
};

// TODO(Ben): Can lock two chunks without deadlock worry with checkerboard pattern updates.

class Chunk {
    friend class ChunkAccessor;
    friend class ChunkGenerator;
    friend class ChunkGrid;
    friend class ChunkMeshManager;
    friend class ChunkMeshTask;
    friend class PagedChunkAllocator;
    friend class SphericalVoxelComponentUpdater;
public:
    
    Chunk():genLevel(ChunkGenLevel::GEN_NONE), pendingGenLevel(ChunkGenLevel::GEN_NONE), isAccessible(false), accessor(nullptr), m_inLoadRange(false), m_handleState(0), m_handleRefCount(0) {}
    // Initializes the chunk but does not set voxel data
    // Should be called after ChunkAccessor sets m_id
    void init(WorldCubeFace face);
    // Initializes the chunk and sets all voxel data to 0
    void initAndFillEmpty(WorldCubeFace face, vvox::VoxelStorageState = vvox::VoxelStorageState::INTERVAL_TREE);
    void setRecyclers(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler);
    void updateContainers();

    /************************************************************************/
    /* Getters                                                              */
    /************************************************************************/
    const ChunkPosition3D& getChunkPosition() const { return m_chunkPosition; }
    const VoxelPosition3D& getVoxelPosition() const { return m_voxelPosition; }
    const ChunkID& getID() const { return m_id; }

    inline ui16 getBlockData(int c) const {
        return blocks.get(c);
    }
    inline ui16 getTertiaryData(int c) const {
        return tertiary.get(c);
    }
    void setBlock(int x, int y, int z, ui16 id) {
        blocks.set(x + y * CHUNK_LAYER + z * CHUNK_WIDTH, id);
    }

    // Marks the chunks as dirty and flags for a re-mesh
    void flagDirty() { isDirty = true; }

    /************************************************************************/
    /* Members                                                              */
    /************************************************************************/
    // Everything else uses this grid data handle
    ChunkGridData* gridData = nullptr;
    MetaFieldInformation meta;
    union {
        struct {
            ChunkHandle left;
            ChunkHandle right;
            ChunkHandle bottom;
            ChunkHandle top;
            ChunkHandle back;
            ChunkHandle front;
        } neighbor;
        ChunkHandle neighbors[6];
    };
    volatile ChunkGenLevel genLevel;
    ChunkGenLevel pendingGenLevel;
    bool isDirty;
    f32 distance2; //< Squared distance
    int numBlocks;
    // TODO(Ben): reader/writer lock
    std::mutex dataMutex;

    volatile bool isAccessible;

    // TODO(Ben): Think about data locality.
    vvox::SmartVoxelContainer<ui16> blocks;
    vvox::SmartVoxelContainer<ui16> tertiary;
    // Block indexes where flora must be generated.
    std::vector<ui16> floraToGenerate;
    volatile ui32 updateVersion;

    ChunkAccessor* accessor;

    static Event<ChunkHandle&> DataChange;
private:
    // For generation
    ChunkGenQueryData m_genQueryData;

    // ui32 m_loadingNeighbors = 0u; ///< Seems like a good idea to get rid of isAccesible
    ChunkPosition3D m_chunkPosition;
    VoxelPosition3D m_voxelPosition;

    ui32 m_activeIndex; ///< Position in active list for m_chunkGrid
    bool m_inLoadRange;

    ChunkID m_id;

    /************************************************************************/
    /* Chunk Handle Data                                                    */
    /************************************************************************/
    std::mutex m_handleMutex;
    ALIGNED_(4) volatile ui32 m_handleState;
    ALIGNED_(4) volatile ui32 m_handleRefCount;
};

#endif // NChunk_h__
