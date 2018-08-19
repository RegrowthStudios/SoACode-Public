//
// VoxelNodeSetterTask.h
// Seed of Andromeda
//
// Created by Benjamin Arnold on 10 Sep 2015
// Copyright 2014 Regrowth Studios
// MIT License
//
// Summary:
// A task for setting voxels using the threadpool.
//

#pragma once

#ifndef VoxelNodeSetterTask_h__
#define VoxelNodeSetterTask_h__

#include <Vorb/IThreadPoolTask.h>
#include "ChunkHandle.h"

class WorkerData;

struct VoxelToPlace {
    VoxelToPlace() {};
    VoxelToPlace(ui16 blockID, ui16 blockIndex) : blockID(blockID), blockIndex(blockIndex) {};
    ui16 blockID;
    ui16 blockIndex;
};

class VoxelNodeSetterTask : public vcore::IThreadPoolTask<WorkerData> {
public:
    // Executes the task
    void execute(WorkerData* workerData) override;

    void cleanup() override;

    ChunkHandle h;
    std::vector<VoxelToPlace> forcedNodes; ///< Always added
    std::vector<VoxelToPlace> condNodes; ///< Conditionally added
};

#endif // VoxelNodeSetterTask_h__
