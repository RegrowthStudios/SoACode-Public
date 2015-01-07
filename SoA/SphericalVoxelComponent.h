///
/// SphericalVoxelComponent.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 5 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Component for voxels mapped to a spherical world
///

#pragma once

#ifndef SphericalVoxelComponent_h__
#define SphericalVoxelComponent_h__

#include "IVoxelMapper.h"

class Camera;
class Chunk;
class ChunkIOManager;
class ChunkManager;
class ParticleEngine;
class PhysicsEngine;
class PlanetGenData;
class SphericalTerrainGenerator;

class SphericalVoxelComponent {
public:
    SphericalVoxelComponent();
    ~SphericalVoxelComponent();

    void initialize(const glm::dvec3 &gpos, vvox::VoxelMapData* startingMapData, GLuint flags);
    void update(const Camera* camera);
    void getClosestChunks(glm::dvec3 &coord, Chunk **chunks);
    void endSession();

    inline ChunkManager* getChunkManager() { return m_chunkManager; }
    inline PhysicsEngine* getPhysicsEngine() { return m_physicsEngine; }

private:
    void updatePhysics(const Camera* camera);

    //chunk manager manages and updates the chunk grid
    ChunkManager* m_chunkManager = nullptr;
    ChunkIOManager* m_chunkIo = nullptr;
    PhysicsEngine* m_physicsEngine = nullptr;
    ParticleEngine* m_particleEngine = nullptr;
    SphericalTerrainGenerator* m_generator = nullptr;

    PlanetGenData* m_planetGenData = nullptr;
};

#endif // SphericalVoxelComponent_h__