///
/// ChunkMeshManager.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 26 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Handles the updating and deallocation of
/// chunk meshes.
///

#pragma once

#ifndef ChunkMeshManager_h__
#define ChunkMeshManager_h__

class ChunkMesh;
class ChunkMeshData;

#include "concurrentqueue.h"
#include <mutex>

class ChunkMeshManager {
public:
    ChunkMeshManager(ui32 startMeshes = 128);
    ~ChunkMeshManager();
    /// Updates the meshManager, uploading any needed meshes
    void update(const f64v3& cameraPosition, bool shouldSort);
    /// Deletes and removes a mesh
    void deleteMesh(ChunkMesh* mesh, int index = -1);
    /// Adds a mesh for updating
    void addMeshForUpdate(ChunkMeshData* meshData) { m_meshQueue.enqueue(meshData); }
    /// Destroys all meshes
    void destroy();

    const std::vector <ChunkMesh *>& getChunkMeshes() { return m_activeChunkMeshes; }

private:
    VORB_NON_COPYABLE(ChunkMeshManager);

    std::mutex m_fmLock; ///< For freeMeshes
    /// Uploads a mesh and adds to list if needed
    void updateMesh(ChunkMeshData* meshData);

    void updateMeshDistances(const f64v3& cameraPosition);

    void updateMeshStorage();

    std::vector <ChunkMesh*> m_activeChunkMeshes;
    std::vector <ChunkMeshData*> m_updateBuffer;
    moodycamel::ConcurrentQueue<ChunkMeshData*> m_meshQueue;

    std::vector <ui32> m_freeMeshes;
    std::vector <ChunkMesh> m_meshStorage;
};

#endif // ChunkMeshManager_h__
