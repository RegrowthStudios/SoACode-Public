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

class Camera;
class NChunk;
class FloraTask;
class Frustum;
class GameSystem;
class GenerateTask;
class GeneratedTreeNodes;
class RenderTask;
class NChunkGrid;
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

    void updateChunks(NChunkGrid& grid, const VoxelPosition3D& agentPosition);

    void tryLoadChunkNeighbors(NChunk* chunk, const VoxelPosition3D& agentPosition, f32 loadDist2);

    void tryLoadChunkNeighbor(const VoxelPosition3D& agentPosition, f32 loadDist2, const f64v3& pos);

};

#endif // SphericalVoxelComponentUpdater_h__