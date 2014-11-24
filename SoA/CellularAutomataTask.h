///
/// CellularAutomataTask.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 24 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Implements the celluar automata task for multithreaded physics
///

#pragma once

#ifndef CellularAutomataTask_h__
#define CellularAutomataTask_h__

#include "IThreadPoolTask.h"

class Chunk;
class WorkerData;

enum CA_FLAG {
    CA_FLAG_LIQUID = 1,
    CA_FLAG_POWDER = 2 
};

class CellularAutomataTask : public vcore::IThreadPoolTask {
public:
    /// Constructs the task
    /// @param chunk: The the chunk to update
    /// @param flags: Combination of CA_FLAG
    CellularAutomataTask(Chunk* chunk, ui32 flags);

    /// Executes the task
    void execute(WorkerData* workerData) override;
private:
    ui32 flags; ///< Flags that tell us what to update
    Chunk* _chunk; ///< The chunk we are updating
};

#endif // CellularAutomataTask_h__