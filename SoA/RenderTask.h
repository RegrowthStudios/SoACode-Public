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

#include "Constants.h"
#include "IThreadPoolTask.h"

class Chunk;
class ChunkGridData;
class ChunkMeshData;

// Sizes For A Padded Chunk
const int PADDED_CHUNK_WIDTH = (CHUNK_WIDTH + 2);
const int PADDED_CHUNK_LAYER = (PADDED_CHUNK_WIDTH * PADDED_CHUNK_WIDTH);
const int PADDED_CHUNK_SIZE = (PADDED_CHUNK_LAYER * PADDED_CHUNK_WIDTH);

enum class MeshJobType { DEFAULT, LIQUID };

#define RENDER_TASK_ID 0

// Represents A Mesh Creation Task
class RenderTask : public vcore::IThreadPoolTask {
public:
    RenderTask() : vcore::IThreadPoolTask(true, RENDER_TASK_ID) {}
    // Executes the task
    void execute(vcore::WorkerData* workerData) override;
    // Helper Function To Set The Chunk Data
    void setChunk(Chunk* ch, MeshJobType cType);

    // Notice that the edges of the chunk data are padded. We have to do this because
    // We don't want to have to access neighboring chunks in multiple threads, that requires
    // us to use mutex locks. This way is much faster and also prevents bounds checking
    ui16 chData[PADDED_CHUNK_SIZE];
    ui16 chLampData[PADDED_CHUNK_SIZE];
    ui8 chSunlightData[PADDED_CHUNK_SIZE];
    ui16 chTertiaryData[PADDED_CHUNK_SIZE];
    ChunkGridData* chunkGridData;
	i32 wSize;
	ui16 wvec[CHUNK_SIZE];
	i32 num;
    MeshJobType type; 
	i32v3 position;
    Chunk* chunk;
    ChunkMeshData* chunkMeshData;
    int levelOfDetail;
};

#endif // RenderTask_h__