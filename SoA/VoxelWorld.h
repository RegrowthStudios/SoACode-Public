#pragma once
#include <Vorb/ThreadPool.h>
#include <Vorb/Vorb.h>

#include "IVoxelMapper.h"
#include "WorldStructs.h"

class Camera;
class ChunkIOManager;
class ChunkManager;
class Item;
class ParticleEngine;
class PhysicsEngine;
class VoxelEditor;

class VoxelWorld
{
public:
    VoxelWorld();
    ~VoxelWorld();
    void initialize(const glm::dvec3 &gpos, vvox::VoxelMapData* startingMapData, GLuint flags);
    void update(const Camera* camera);
    void getClosestChunks(glm::dvec3 &coord, class Chunk **chunks);
    void endSession();

    inline ChunkManager &getChunkManager() { return *m_chunkManager; }
    inline PhysicsEngine* getPhysicsEngine() { return m_physicsEngine; }

private:
    void updatePhysics(const Camera* camera);

    //chunk manager manages and updates the chunk grid
    ChunkManager* m_chunkManager = nullptr;
    ChunkIOManager* m_chunkIo = nullptr;
    PhysicsEngine* m_physicsEngine = nullptr;
    ParticleEngine* m_particleEngine = nullptr;
};

