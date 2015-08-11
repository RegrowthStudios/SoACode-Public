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

class Chunk;
typedef Chunk* ChunkPtr;

class ChunkGridData {
public:
    ChunkGridData() {};
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
    friend class ChunkAccessor;
    friend class ChunkGenerator;
    friend class ChunkGrid;
    friend class ChunkMeshManager;
    friend class ChunkMeshTask;
    friend class PagedChunkAllocator;
    friend class SphericalVoxelComponentUpdater;
public:
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

    // True when the chunk needs to be meshed
    bool needsRemesh() { return remeshFlags != 0; }
    // Marks the chunks as dirty and flags for a re-mesh
    void flagDirty() { isDirty = true; remeshFlags |= 1; }

    /************************************************************************/
    /* Members                                                              */
    /************************************************************************/
    // Everything else uses this grid data handle
    ChunkGridData* gridData = nullptr;
    PlanetHeightData* heightData = nullptr;
    MetaFieldInformation meta;
    union {
        UNIONIZE(ChunkHandle left;
                 ChunkHandle right;
                 ChunkHandle bottom;
                 ChunkHandle top;
                 ChunkHandle back;
                 ChunkHandle front);
        UNIONIZE(ChunkHandle neighbors[6]);
    };
    ChunkGenLevel genLevel = ChunkGenLevel::GEN_NONE;
    ChunkGenLevel pendingGenLevel = ChunkGenLevel::GEN_NONE;
    bool isDirty;
    f32 distance2; //< Squared distance
    int numBlocks;
    std::mutex dataMutex;

    ui8 remeshFlags;
    volatile bool isAccessible = false;
    volatile bool queuedForMesh = false;

    // TODO(Ben): Think about data locality.
    vvox::SmartVoxelContainer<ui16> blocks;
    vvox::SmartVoxelContainer<ui16> tertiary;
    // Block indexes where flora must be generated.
    std::vector<ui16> floraToGenerate;

    ChunkAccessor* accessor = nullptr;
private:
    // For generation
    ChunkGenQueryData m_genQueryData;

    // ui32 m_loadingNeighbors = 0u; ///< Seems like a good idea to get rid of isAccesible
    ChunkPosition3D m_chunkPosition;
    VoxelPosition3D m_voxelPosition;

    ui32 m_activeIndex; ///< Position in active list for m_chunkGrid
    bool m_inLoadRange = false;

    ChunkID m_id;

    /************************************************************************/
    /* Chunk Handle Data                                                    */
    /************************************************************************/
    std::mutex m_handleMutex;
    __declspec(align(4)) volatile ui32 m_handleState = 0;
    __declspec(align(4)) volatile ui32 m_handleRefCount = 0;
};

#endif // NChunk_h__
