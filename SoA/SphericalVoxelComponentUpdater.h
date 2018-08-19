///
/// SphericalVoxelComponentUpdater.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 8 Jan 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
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
    void updateComponent(SphericalVoxelComponent& cmp);

    void updateChunks(ChunkGrid& grid, bool doGen);

    SphericalVoxelComponent* m_cmp = nullptr; ///< Component we are updating
};

#endif // SphericalVoxelComponentUpdater_h__
