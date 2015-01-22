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

struct AxisRotationComponent;
class Camera;
class Chunk;
class GameSystem;
struct NamePositionComponent;
class SoaState;
class SpaceSystem;
struct SphericalVoxelComponent;

class SphericalVoxelComponentUpdater {
public:
    void update(SpaceSystem* spaceSystem, const GameSystem* gameSystem, const SoaState* soaState);
    void getClosestChunks(SphericalVoxelComponent* cmp, glm::dvec3 &coord, Chunk **chunks);
    void endSession(SphericalVoxelComponent* cmp);

    /// Updates openGL specific stuff. Call on render thread
    void glUpdate(SpaceSystem* spaceSystem);
private:
    void destroyVoxels();
    void updatePhysics(const Camera* camera);
};

#endif // SphericalVoxelComponentUpdater_h__