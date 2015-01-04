// 
//  MeshManager.h
//  Seed Of Andromeda
//
//  Created by Ben Arnold on 19 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides a mesh manager class, which handles
//  uploading various types of meshes to the GPU.
//

#pragma once

#ifndef MESHMANAGER_H_
#define MESHMANAGER_H_

class ChunkMeshData;
class ParticleMeshMessage;
class PhysicsBlockMeshMessage;

class ChunkMesh;
class ParticleMesh;
class PhysicsBlockMesh;
class SphericalTerrainMesh;

class MeshManager
{
public:
    MeshManager();

    /// Updates a chunk mesh
    /// @param cmd: the ChunkMeshData sent by the update thread
    void updateChunkMesh(ChunkMeshData* cmd);

    /// Updates a particle mesh
    /// @param pmm: The ParticleMeshMessage sent by the update thread
    void updateParticleMesh(ParticleMeshMessage* pmm);

    /// Updates a physics block mesh
    /// @param pbmm: The PhysicsBlockMeshMessage sent by the update thread
    void updatePhysicsBlockMesh(PhysicsBlockMeshMessage* pbmm);

    /// Sorts the messages from front to back
    /// @param cameraPosition: The position of the camera to calculate distance from
    void sortMeshes(const f64v3& cameraPosition);

    /// Destroys all of the meshes and frees allocated memory
    void destroy();

    // Getters
    const std::vector <ChunkMesh*>& getChunkMeshes() const { return _chunkMeshes; }
    const std::vector <ParticleMesh*>& getParticleMeshes() const { return _particleMeshes; }
    const std::vector <PhysicsBlockMesh*>& getPhysicsBlockMeshes() const { return _physicsBlockMeshes; }

private:

    /// Updates the distance field for the meshes
    /// @param cameraPosition: The position of the camera to calculate distance from
    void updateMeshDistances(const f64v3& cameraPosition);

    /// Sorts a mesh list recursively with quicksort
    /// @param v: the chunk list
    /// @param start: the start for this sort
    /// @param size: the size of this block
    void recursiveSortMeshList(std::vector <ChunkMesh*> &v, int start, int size);

    std::vector <ChunkMesh *> _chunkMeshes;
    std::vector <ParticleMesh *> _particleMeshes;
    std::vector <PhysicsBlockMesh *> _physicsBlockMeshes;

};

#endif // MESHMANAGER_H_