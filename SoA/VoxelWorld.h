#pragma once
#include "ThreadPool.h"
#include "WorldStructs.h"

#include "Vorb.h"
#include "IVoxelMapper.h"

class Camera;
class ChunkManager;
class VoxelEditor;
class Item;

class VoxelWorld
{
public:
    VoxelWorld();
    ~VoxelWorld();
    void initialize(const glm::dvec3 &gpos, vvox::VoxelMapData* startingMapData, GLuint flags);
    void update(const Camera* camera);
    void getClosestChunks(glm::dvec3 &coord, class Chunk **chunks);
    void endSession();

    inline ChunkManager &getChunkManager() { return *_chunkManager; }

private:
    void updatePhysics(const Camera* camera);

    //chunk manager manages and updates the chunk grid
    ChunkManager* _chunkManager;
};

