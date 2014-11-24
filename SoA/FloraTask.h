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

class GeneratedTreeNodes {
public:
    /// Try to place blocks roughly once per FRAMES_BEFORE_ATTEMPT frames
    static const int FRAMES_BEFORE_ATTEMPT = 40;

    int numFrames = 0; ///< Number of frames left before placement attempt
    i32v3 startChunkGridPos; ///< Grid position of initial chunk
    std::vector <TreeNode> wnodes; ///< Wood nodes
    std::vector <TreeNode> lnodes; ///< Leaf nodes
    std::vector <i32v3> allChunkPositions; ///< Stores positions of all chunks
};

class FloraTask : public vcore::IThreadPoolTask {
public:
    FloraTask() : vcore::IThreadPoolTask(true, FLORA_TASK_ID) {}

    /// Executes the task
    virtual void execute(vcore::WorkerData* workerData) override;

    /// Initializes the task
    void init(Chunk* ch) { chunk = ch; }

    /// Helper Function
    static i32v3 getChunkOffset(ui16 offset);

    bool isSuccessful; ///< True if the generation finished
    Chunk* chunk = nullptr;

    GeneratedTreeNodes* generatedTreeNodes;
private:
    /// Sorts TreeNode vectors based on chunk
    /// @param generatedTreeNodes: The wrapper for the data to sort
    void sortNodes(GeneratedTreeNodes* generatedTreeNodes);
};

#endif // FloraTask_h__