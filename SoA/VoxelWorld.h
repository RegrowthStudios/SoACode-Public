#pragma once
#include "ThreadPool.h"
#include "WorldStructs.h"

#include "Vorb.h"
#include "IVoxelMapper.h"

class ChunkManager;
class Planet;
class VoxelEditor;
class Item;

class VoxelWorld
{
public:
    VoxelWorld();
    ~VoxelWorld();
    void initialize(const glm::dvec3 &gpos, vvoxel::VoxelMapData* startingMapData, Planet *planet, GLuint flags);
    void beginSession(const glm::dvec3 &gridPosition);
    void update(const glm::dvec3 &position, const glm::dvec3 &viewDir);
    void closeThreadPool();
    int getClosestChunks(glm::dvec3 &coord, class Chunk **chunks);
    void endSession();

    void setPlanet(class Planet *planet);

    Planet *getPlanet() { return _planet; }

    inline ChunkManager &getChunkManager() { return *_chunkManager; }

private:
    void initializeThreadPool();

    //the planet associated with this world
    Planet* _planet;

    //chunk manager manages and updates the chunk grid
    ChunkManager* _chunkManager;
};

