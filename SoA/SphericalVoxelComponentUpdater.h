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

    void updateComponent(SphericalVoxelComponent& svc, const f64v3& position, const Frustum* frustum);

    /// Updates all chunks
    /// @param position: the observer position
    /// @param frustum: The view frustum of the observer
    void updateChunks(SphericalVoxelComponent& svc, const f64v3& position, const Frustum* frustum);

    void updatePhysics(const Camera* camera);

    /// Updates all chunks that have been loaded
    void updateLoadedChunks(ui32 maxTicks);

    /// Updates all chunks that are ready to be generated
    void updateGenerateList();

    /// Updates the treesToPlace list
    /// @param maxTicks: maximum time the function is allowed
    void updateTreesToPlace(ui32 maxTicks);

    /// Updates the load list
    /// @param maxTicks: maximum time the function is allowed
    void updateLoadList(ui32 maxTicks);

    /// Updates the setup list
    /// @param maxTicks: maximum time the function is allowed
    i32 updateSetupList(ui32 maxTicks);

    /// Updates the mesh list
    /// @param maxTicks: maximum time the function is allowed
    i32 updateMeshList(ui32 maxTicks);
};

#endif // SphericalVoxelComponentUpdater_h__