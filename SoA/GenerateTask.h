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

#include "IThreadPoolTask.h"

class Chunk;
struct LoadData;

#define GENERATE_TASK_ID 1

// Represents A Chunk Load Task
struct GenerateTask : public IThreadPoolTask {
public:
    GenerateTask() : IThreadPoolTask(true, GENERATE_TASK_ID) { }

    void init(Chunk *ch = 0, LoadData *ld = 0) {
        chunk = ch;
        loadData = ld;
    }

    void execute(WorkerData* workerData) override;

    // Chunk To Be Loaded
    Chunk* chunk;

    // Loading Information
    LoadData* loadData;

   

};

#endif // LoadTask_h__