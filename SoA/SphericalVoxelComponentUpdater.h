///
/// SphericalVoxelComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Updates spherical voxel components
///

#pragma once

#ifndef SphericalVoxelComponentUpdater_h__
#define SphericalVoxelComponentUpdater_h__

#include "ChunkHandle.h"

class Camera;
class Chunk;
class FloraTask;
class Frustum;
class GameSystem;
class GenerateTask;
class GeneratedTreeNodes;
class ChunkMeshTask;
class ChunkGrid;
struct SoaState;
class SpaceSystem;
struct AxisRotationComponent;
struct NamePositionComponent;
struct SphericalVoxelComponent;
struct VoxelPosition3D;

#include "VoxelCoordinateSpaces.h"

class SphericalVoxelComponentUpdater {
public:
    void update(const SoaState* soaState);

private:

    SphericalVoxelComponent* m_cmp = nullptr; ///< Component we are updating

    void updateComponent(const VoxelPosition3D& agentPosition);

    void updateChunks(ChunkGrid& grid, const VoxelPosition3D& agentPosition, bool doGen);

    void requestChunkMesh(ChunkHandle chunk);

    void disposeChunkMesh(Chunk* chunk);

    ChunkMeshTask* trySetMeshDependencies(ChunkHandle chunk);

    i32v3 m_lastChunkPos = i32v3(INT_MAX);
    f64v3 m_lastAgentPos = f64v3(FLT_MAX);
};

#endif // SphericalVoxelComponentUpdater_h__
