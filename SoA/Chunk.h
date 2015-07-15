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

// A flora/tree that needs to be generated
struct QueuedFlora {
    QueuedFlora(ui16 floraID, ui16 blockIndex) : floraID(floraID), blockIndex(blockIndex) {}
    ui16 floraID;
    ui16 blockIndex;
};

class Chunk {
    friend class ChunkGenerator;
    friend class ChunkGrid;
    friend class ChunkMeshTask;
public:
    // Initializes the chunk but does not set voxel data
    void init(ChunkID id, const ChunkPosition3D& pos);
    // Initializes the chunk and sets all voxel data to 0
    void initAndFillEmpty(ChunkID id, const ChunkPosition3D& pos, vvox::VoxelStorageState = vvox::VoxelStorageState::INTERVAL_TREE);
    void setRecyclers(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler);
    void updateContainers();

    /************************************************************************/
    /* Getters                                                              */
    /************************************************************************/
    const ChunkPosition3D& getChunkPosition() const { return m_chunkPosition; }
    const VoxelPosition3D& getVoxelPosition() const { return m_voxelPosition; }
    bool hasAllNeighbors() const { return numNeighbors == 6u; }
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
    ChunkGenLevel genLevel = ChunkGenLevel::GEN_NONE;
    bool hasCreatedMesh = false;
    bool isDirty;
    bool isInRange;
    f32 distance2; //< Squared distance
    int numBlocks;
    volatile int refCount; ///< Only change on main thread
    std::mutex mutex;

    ui32 numNeighbors = 0u;
    ui8 remeshFlags;
    volatile bool isAccessible = false;
    volatile bool queuedForMesh = false;

    // TODO(Ben): Think about data locality.
    vvox::SmartVoxelContainer<ui16> blocks;
    vvox::SmartVoxelContainer<ui16> tertiary;
    std::vector<QueuedFlora> floraToGenerate;
private:
    // For generation
    ChunkGenQueryData m_genQueryData;

    ui32 m_loadingNeighbors = 0u; ///< Seems like a good idea to get rid of isAccesible
    ChunkPosition3D m_chunkPosition;
    VoxelPosition3D m_voxelPosition;
  
    ChunkID m_id;
};

#endif // NChunk_h__
