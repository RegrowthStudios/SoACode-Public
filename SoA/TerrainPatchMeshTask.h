///
/// TerrainPatchMeshTask.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 1 Jul 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Task for terrain patch mesh
///

#pragma once

#ifndef TerrainPatchMeshTask_h__
#define TerrainPatchMeshTask_h__

#include <Vorb/IThreadPoolTask.h>

#include "Constants.h"
#include "VoxPool.h"
#include "VoxelCoordinateSpaces.h"

struct TerrainPatchData;
class TerrainPatchMesh;
class TerrainPatchMesher;
class TerrainPatchMeshManager;
class SphericalHeightmapGenerator;

#define TERRAIN_MESH_TASK_ID 6

// Represents A Mesh Creation Task
class TerrainPatchMeshTask : public vcore::IThreadPoolTask < WorkerData > {
public:
    TerrainPatchMeshTask() : vcore::IThreadPoolTask<WorkerData>(TERRAIN_MESH_TASK_ID) {}

    // Initializes the task
    void init(const TerrainPatchData* patchData,
              TerrainPatchMesh* mesh,
              const f32v3& startPos,
              float width,
              WorldCubeFace cubeFace);

    // Executes the task
    void execute(WorkerData* workerData) override;

private:
    f32v3 m_startPos;
    WorldCubeFace m_cubeFace;
    float m_width;
    TerrainPatchMesh* m_mesh = nullptr;
    const TerrainPatchData* m_patchData = nullptr;
};

#endif // TerrainPatchMeshTask_h__
