///
/// FloraTask.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 18 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Implements the flora generation task for SoA
///

#pragma once

#ifndef FloraTask_h__
#define FloraTask_h__

#include "IThreadPoolTask.h"
#include "FloraGenerator.h"

class WorkerData;
class Chunk;

#define FLORA_TASK_ID 2

class FloraTask : public vcore::IThreadPoolTask {
public:
    FloraTask() : vcore::IThreadPoolTask(true, FLORA_TASK_ID) {}

    // Executes the task
    virtual void execute(vcore::WorkerData* workerData) override;

    // Initializes the task
    void init(Chunk* ch) { chunk = ch; }

    bool isSuccessful; ///< True if the generation finished
    Chunk* chunk = nullptr;

    std::vector <TreeNode> wnodes; ///< Wood nodes
    std::vector <TreeNode> lnodes; ///< Leaf nodes
};

#endif // FloraTask_h__