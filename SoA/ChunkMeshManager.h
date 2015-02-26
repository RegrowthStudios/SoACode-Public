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

class ChunkMeshManager {
public:
    ChunkMeshManager();
    ~ChunkMeshManager();

    /// Uploads a mesh and adds for tracking if needed
    void uploadMesh(ChunkMeshData* meshData);
    /// Deletes and removes a mesh
    void deleteMesh(ChunkMesh* mesh);

private:
    std::vector <ChunkMesh *> m_chunkMeshes;
};

#endif // ChunkMeshManager_h__
