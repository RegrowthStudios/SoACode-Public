///
/// RenderTask.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// This file has the implementation of a render task for SoA
///

#pragma once

#ifndef RenderTask_h__
#define RenderTask_h__

#include <Vorb/IThreadPoolTask.h>

#include "ChunkHandle.h"
#include "Constants.h"
#include "VoxPool.h"

class Chunk;
class ChunkGridData;
class ChunkMesh;
class ChunkMeshData;
class ChunkMeshManager;
class VoxelLightEngine;
class BlockPack;

enum class MeshTaskType { DEFAULT, LIQUID };

enum MeshNeighborHandles {
    NEIGHBOR_HANDLE_LEFT = 0,
    NEIGHBOR_HANDLE_RIGHT,
    NEIGHBOR_HANDLE_FRONT,
    NEIGHBOR_HANDLE_BACK,
    NEIGHBOR_HANDLE_TOP,
    NEIGHBOR_HANDLE_BOT,
    NUM_NEIGHBOR_HANDLES ///< Has to be last
};

#define CHUNK_MESH_TASK_ID 0

// Represents A Mesh Creation Task
class ChunkMeshTask : public vcore::IThreadPoolTask<WorkerData> {
public:
    ChunkMeshTask() : vcore::IThreadPoolTask<WorkerData>(CHUNK_MESH_TASK_ID) {}

    // Executes the task
    void execute(WorkerData* workerData) override;

    // Initializes the task
    void init(ChunkHandle& ch, MeshTaskType cType, const BlockPack* blockPack, ChunkMeshManager* meshManager);

    MeshTaskType type; 
    ChunkHandle chunk;
    ChunkMeshManager* meshManager = nullptr;
    const BlockPack* blockPack = nullptr;
    ChunkHandle neighborHandles[NUM_NEIGHBOR_HANDLES];
private:
    void updateLight(VoxelLightEngine* voxelLightEngine);
};

#endif // RenderTask_h__
