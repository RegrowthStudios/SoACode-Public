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

    void updateComponent(const f64v3& position, const Frustum* frustum);

    /// Updates all chunks
    /// @param position: the observer position
    /// @param frustum: The view frustum of the observer
    void updateChunks(const f64v3& position, const Frustum* frustum);

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

    /// Creates a chunk and any needed grid data at a given chunk position
    /// @param chunkPosition: the ChunkPosition
    void makeChunkAt(const ChunkPosition3D& chunkPosition);

    /// Gets all finished tasks from threadpool
    void processFinishedTasks();

    /// Processes a generate task that is finished
    void processFinishedGenerateTask(GenerateTask* task);

    /// Processes a flora task that is finished
    void processFinishedFloraTask(FloraTask* task);

    /// Adds a generate task to the threadpool
    void addGenerateTask(NChunk* chunk);

    /// Places a batch of tree nodes
    /// @param nodes: the nodes to place
    void placeTreeNodes(GeneratedTreeNodes* nodes);

    /// Frees a chunk from the world. 
    /// The chunk may be recycled, or it may need to wait for some threads
    /// to finish processing on it.
    /// @param chunk: the chunk to free
    void freeChunk(NChunk* chunk);

    /// Updates the neighbors for a chunk, possibly loading new chunks
    /// @param chunk: the chunk in question
    /// @param cameraPos: camera position
    void updateChunkNeighbors(NChunk* chunk, const i32v3& cameraPos);

    /// Tries to load a chunk neighbor if it is in range
    /// @param chunk: relative chunk
    /// @param cameraPos: the camera position
    /// @param offset: the offset, must be unit length.
    void tryLoadChunkNeighbor(NChunk* chunk, const i32v3& cameraPos, const i32v3& offset);

    /// True when the chunk can be sent to mesh thread. Will set neighbor dependencies
    bool trySetMeshDependencies(NChunk* chunk);

    /// Removes all mesh dependencies
    void tryRemoveMeshDependencies(NChunk* chunk);
};

#endif // SphericalVoxelComponentUpdater_h__