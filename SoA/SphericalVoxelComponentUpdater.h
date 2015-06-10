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
struct SoaState;
class SpaceSystem;
struct AxisRotationComponent;
struct NamePositionComponent;
struct SphericalVoxelComponent;

#include "VoxelCoordinateSpaces.h"

class SphericalVoxelComponentUpdater {
public:
    void update(const SoaState* soaState);

private:

    SphericalVoxelComponent* m_cmp = nullptr; ///< Component we are updating

    void updateComponent(const f64v3& position);

};

#endif // SphericalVoxelComponentUpdater_h__