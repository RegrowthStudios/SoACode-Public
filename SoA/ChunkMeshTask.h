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

class Chunk;
class ChunkGridData;
class ChunkMesh;
class ChunkMeshData;
class ChunkMeshManager;
class VoxelLightEngine;
class BlockPack;

enum class MeshTaskType { DEFAULT, LIQUID };

#define RENDER_TASK_ID 0

// Represents A Mesh Creation Task
class ChunkMeshTask : public vcore::IThreadPoolTask<WorkerData> {
public:
    ChunkMeshTask() : vcore::IThreadPoolTask<WorkerData>(false, RENDER_TASK_ID) {}

    // Executes the task
    void execute(WorkerData* workerData) override;

    // Initializes the task
    void init(Chunk* ch, MeshTaskType cType, const BlockPack* blockPack, ChunkMeshManager* meshManager);

    MeshTaskType type; 
    Chunk* chunk = nullptr;
    ChunkMeshManager* meshManager = nullptr;
    const BlockPack* blockPack = nullptr;
private:
    void updateLight(VoxelLightEngine* voxelLightEngine);
    void setupMeshData(ChunkMesher* chunkMesher);
};

#endif // RenderTask_h__
