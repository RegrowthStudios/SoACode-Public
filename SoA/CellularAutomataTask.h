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

#include <Vorb/IThreadPoolTask.h>

#include "VoxPool.h"

class CaPhysicsType;
class Chunk;
class ChunkManager;
class ChunkMeshManager;
class PhysicsEngine;
class RenderTask;

#define CA_TASK_ID 3

enum class CA_ALGORITHM {
    NONE = 0,
    LIQUID = 1,
    POWDER = 2 
};

class CellularAutomataTask : public vcore::IThreadPoolTask<WorkerData> {
public:
    friend class ChunkManager;
    friend class SphericalVoxelComponentUpdater;
    /// Constructs the task
    CellularAutomataTask(ChunkManager* chunkManager,
                         PhysicsEngine* physicsEngine,
                         Chunk* chunk,
                         OPT ChunkMeshManager* meshManager);

    /// Adds a caPhysicsType for update
    void addCaTypeToUpdate(CaPhysicsType* caType) {
        typesToUpdate.push_back(caType);
    }

    /// Executes the task
    void execute(WorkerData* workerData) override;

    RenderTask* renderTask = nullptr; ///< A nested to force re-mesh

private:
    std::vector<CaPhysicsType*> typesToUpdate; ///< All the CA types that will be updated by this task
    Chunk* _chunk = nullptr; ///< The chunk we are updating
    ChunkManager* m_chunkManager = nullptr;
    PhysicsEngine* m_physicsEngine = nullptr;
    ChunkMeshManager* meshManager = nullptr;
};

#endif // CellularAutomataTask_h__