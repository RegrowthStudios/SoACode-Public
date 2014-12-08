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

#include <IThreadPoolTask.h>

#include "VoxPool.h"

class Chunk;
class RenderTask;
class CaPhysicsType;

#define CA_TASK_ID 3

enum class CA_ALGORITHM {
    NONE = 0,
    LIQUID = 1,
    POWDER = 2 
};

class CellularAutomataTask : public vcore::IThreadPoolTask<WorkerData> {
public:
    friend class ChunkManager;
    /// Constructs the task
    /// @param chunk: The the chunk to update
    /// @param flags: Combination of CA_FLAG
    CellularAutomataTask(Chunk* chunk, bool makeMesh);

    /// Adds a caPhysicsType for update
    void addCaTypeToUpdate(CaPhysicsType* caType) {
        typesToUpdate.push_back(caType);
    }

    /// Executes the task
    void execute(WorkerData* workerData) override;

    RenderTask* renderTask = nullptr; ///< A nested to force re-mesh

private:
    std::vector<CaPhysicsType*> typesToUpdate; ///< All the CA types that will be updated by this task
    Chunk* _chunk; ///< The chunk we are updating
};

#endif // CellularAutomataTask_h__