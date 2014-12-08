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

#include <ThreadPool.h>

// Worker data for a threadPool
class WorkerData {
public:
    ~WorkerData() {
        delete chunkMesher;
        delete floraGenerator;
        delete voxelLightEngine;
        delete caEngine;
    }
    volatile bool waiting;
    volatile bool stop;

    // Each thread gets its own generators
    class ChunkMesher* chunkMesher = nullptr;
    class FloraGenerator* floraGenerator = nullptr;
    class VoxelLightEngine* voxelLightEngine = nullptr;
    class CAEngine* caEngine = nullptr;
};

class VoxPool : public vcore::ThreadPool<WorkerData> {
    // Empty
};

#endif // VoxPool_h__



