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

#include "Constants.h"
#include "VoxPool.h"

class NChunk;
class ChunkGridData;
class ChunkMesh;
class ChunkMeshData;
class ChunkMeshManager;
class VoxelLightEngine;

enum class RenderTaskType { DEFAULT, LIQUID };

#define RENDER_TASK_ID 0

// Represents A Mesh Creation Task
class RenderTask : public vcore::IThreadPoolTask<WorkerData> {
public:
    RenderTask() : vcore::IThreadPoolTask<WorkerData>(true, RENDER_TASK_ID) {}

    // Executes the task
    void execute(WorkerData* workerData) override;

    // Initializes the task
    void init(NChunk* ch, RenderTaskType cType, BlockPack* blockPack, ChunkMeshManager* meshManager);

    RenderTaskType type; 
    NChunk* chunk = nullptr;
    ChunkMeshManager* meshManager = nullptr;
    BlockPack* blockPack = nullptr;
private:
    void updateLight(VoxelLightEngine* voxelLightEngine);
    void setupMeshData(ChunkMesher* chunkMesher);
};

#endif // RenderTask_h__