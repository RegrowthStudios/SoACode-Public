///
/// VoxPool.h
/// Seed of Andromeda
///
/// Created by Cristian Zaloj on 7 Dec 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// 
///

#pragma once

#ifndef VoxPool_h__
#define VoxPool_h__

#include <Vorb/ThreadPool.h>

// Worker data for a threadPool
class WorkerData {
public:
    ~WorkerData();
    volatile bool waiting;
    volatile bool stop;

    // Each thread gets its own generators
    class ChunkMesher* chunkMesher = nullptr;
    class TerrainPatchMesher* terrainMesher = nullptr;
    class FloraGenerator* floraGenerator = nullptr;
    class VoxelLightEngine* voxelLightEngine = nullptr;
};

typedef vcore::ThreadPool<WorkerData> VoxPool;

#endif // VoxPool_h__



