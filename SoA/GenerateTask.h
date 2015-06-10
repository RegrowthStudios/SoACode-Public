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

class NChunk;
class LoadData;

#define GENERATE_TASK_ID 1

// Represents A Chunk Load Task

class GenerateTask : public vcore::IThreadPoolTask<WorkerData> {

public:
    GenerateTask() : vcore::IThreadPoolTask<WorkerData>(true, GENERATE_TASK_ID) {}

    void init(NChunk *ch, LoadData *ld) {
        chunk = ch;
        loadData = ld;
    }

    void execute(WorkerData* workerData) override;

    // Chunk To Be Loaded
    NChunk* chunk;

    // Loading Information
    LoadData* loadData;

};

#endif // LoadTask_h__