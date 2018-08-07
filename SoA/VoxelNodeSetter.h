//
// VoxelNodeSetter.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 9 Sep 2015
// Copyright 2014 Regrowth Studios
// All Rights Reserved
//
// Summary:
// Waits on chunk generation queries and then sets voxels.
//

#pragma once

#ifndef VoxelNodeSetter_h__
#define VoxelNodeSetter_h__

#include <vector>
#include "ChunkQuery.h"
#include "VoxelNodeSetterTask.h"

class ChunkHandle;
class ChunkGrid;

struct VoxelNodeSetterWaitingChunk {
    Chunk* ch;
    ChunkGenLevel requiredGenLevel;
};

struct VoxelNodeSetterLookupData {
    ui32 waitingChunksIndex;
    ChunkHandle h;
    std::vector<VoxelToPlace> forcedNodes; ///< Always added
    std::vector<VoxelToPlace> condNodes; ///< Conditionally added
};

class VoxelNodeSetter {
public:
    // Contents of vectors may be cleared
    void setNodes(ChunkHandle& h,
                  ChunkGenLevel requiredGenLevel,
                  std::vector<VoxelToPlace>& forcedNodes,
                  std::vector<VoxelToPlace>& condNodes);

    void update();

    ChunkGrid* grid = nullptr;
    vcore::ThreadPool<WorkerData>* threadPool;
private:
    std::mutex m_lckVoxelsToAdd;
    std::vector<VoxelNodeSetterWaitingChunk> m_waitingChunks;
    std::map<Chunk*, VoxelNodeSetterLookupData> m_handleLookup; ///< Stores handles since they fk up in vector.
};

#endif // VoxelNodeSetter_h__
