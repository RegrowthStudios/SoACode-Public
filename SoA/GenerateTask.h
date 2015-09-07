///
/// GenerateTask.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Implements the generate task for SoA chunk generation
///

#pragma once

#ifndef LoadTask_h__
#define LoadTask_h__

#include <Vorb/IThreadPoolTask.h>

#include "VoxPool.h"

class Chunk;
class ChunkGenerator;
class ChunkQuery;
struct PlanetHeightData;

#define GENERATE_TASK_ID 1

// Represents A Chunk Load Task

class GenerateTask : public vcore::IThreadPoolTask<WorkerData> {
public:
    GenerateTask() : vcore::IThreadPoolTask<WorkerData>(false, GENERATE_TASK_ID) {}

    void init(ChunkQuery *query,
              PlanetHeightData* heightData,
              ChunkGenerator* chunkGenerator) {
        this->query = query;
        this->heightData = heightData;
        this->chunkGenerator = chunkGenerator;
    }

    void execute(WorkerData* workerData) override;

    // Chunk To Be Loaded
    ChunkQuery* query;
    ChunkGenerator* chunkGenerator; ///< send finished query here

    // Loading Information
    PlanetHeightData* heightData;

private:
    void generateFlora(WorkerData* workerData, Chunk& chunk);
};

#endif // LoadTask_h__
