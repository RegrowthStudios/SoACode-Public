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

class ChunkMeshManager {
public:
    ChunkMeshManager();
    ~ChunkMeshManager();
    /// Updates the meshManager, uploading any needed meshes
    void update();
    /// Deletes and removes a mesh
    void deleteMesh(ChunkMesh* mesh);
    /// Adds a mesh for updating
    void addMeshForUpdate(ChunkMeshData* meshData);
    /// Destroys all meshes
    void destroy();

    const std::vector <ChunkMesh *>& getChunkMeshes() { return m_chunkMeshes; }

private:
    /// Uploads a mesh and adds to list if needed
    void updateMesh(ChunkMeshData* meshData);

    std::vector <ChunkMesh*> m_chunkMeshes;
    std::vector <ChunkMeshData*> m_updateBuffer;
    moodycamel::ConcurrentQueue<ChunkMeshData*> m_meshQueue;
};

#endif // ChunkMeshManager_h__
