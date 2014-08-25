#pragma once
#include "ThreadPool.h"
#include "WorldStructs.h"

class ChunkManager;
class Planet;
class VoxelEditor;
class Item;

class VoxelWorld
{
public:
    VoxelWorld();
    ~VoxelWorld();
    void initialize(const glm::dvec3 &gpos, FaceData *faceData, Planet *planet, bool setYfromHeight, bool flatgrass);
    void beginSession(const glm::dvec3 &gridPosition);
    void update(const glm::dvec3 &position, const glm::dvec3 &viewDir);
    void closeThreadPool();
    void resizeGrid(const glm::dvec3 &gpos);
    int getClosestChunks(glm::dvec3 &coord, class Chunk **chunks);
    void endSession();

    void setPlanet(class Planet *planet);

    int getCenterY() const;

    Planet *getPlanet() { return _planet; }

    inline ChunkManager &getChunkManager() { return *_chunkManager; }

private:
    void initializeThreadPool();

    //the planet associated with this world
    Planet* _planet;

    //faceData stores the info about the current world cube face
    FaceData* _faceData;

    //chunk manager manages and updates the chunk grid
    ChunkManager* _chunkManager;
};

