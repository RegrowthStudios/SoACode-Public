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

#include <vector>

class TerrainMeshMessage;
class ChunkMeshData;
class ParticleMeshMessage;
class PhysicsBlockMeshMessage;

struct ChunkMesh;
struct ParticleMesh;
struct PhysicsBlockMesh;

class MeshManager
{
public:
    MeshManager();
 
    void updateTerrainMesh(TerrainMeshMessage* tmm);
    void updateChunkMesh(ChunkMeshData* cmd);
    void updateParticleMesh(ParticleMeshMessage* pmm);
    void updatePhysicsBlockMesh(PhysicsBlockMeshMessage* pbmm);

private:
    std::vector <ChunkMesh *> _chunkMeshes;
    std::vector <ParticleMesh *> _particleMeshes;
    std::vector <PhysicsBlockMesh *> _physicsBlockMeshes;

};

