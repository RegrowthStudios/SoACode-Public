#include "stdafx.h"
#include "VoxelWorld.h"

#include "VoxelPlanetMapper.h"

#include "BlockData.h"
#include "CAEngine.h"
#include "Chunk.h"
#include "ChunkManager.h"
#include "Errors.h"
#include "GameManager.h"
#include "Item.h"
#include "PhysicsEngine.h"
#include "Planet.h"
#include "Sound.h"
#include "VoxelEditor.h"
#include "global.h"
#include "utils.h"

VoxelWorld::VoxelWorld() : _planet(NULL), _chunkManager(NULL) {

}


VoxelWorld::~VoxelWorld()
{
}


void VoxelWorld::initialize(const glm::dvec3 &gpos, vvoxel::VoxelMapData* startingMapData, Planet *planet, GLuint flags)
{
    if (_chunkManager) {
        pError("VoxelWorld::initialize() called twice before end session!");
        delete _chunkManager;
    }
    _chunkManager = new ChunkManager();
    GameManager::chunkManager = _chunkManager;
    
    if (planet == NULL) showMessage("Initialized chunk manager with NULL planet!");

    _chunkManager->planet = planet;

    vvoxel::VoxelPlanetMapper* voxelPlanetMapper = new vvoxel::VoxelPlanetMapper(planet->facecsGridWidth);
    _chunkManager->initialize(gpos, voxelPlanetMapper, startingMapData, flags);

    setPlanet(planet);
}

void VoxelWorld::update(const glm::dvec3 &position, const glm::dvec3 &viewDir)
{
    // Update the Chunks
    _chunkManager->update(position, viewDir);
    // Update the physics
    updatePhysics(position, viewDir);
}

void VoxelWorld::setPlanet(Planet *planet)
{
    _planet = planet;
    GameManager::planet = planet;
}

void VoxelWorld::getClosestChunks(glm::dvec3 &coord, class Chunk **chunks)
{
    _chunkManager->getClosestChunks(coord, chunks);
}

void VoxelWorld::endSession()
{
    GameManager::chunkIOManager->onQuit();
    _chunkManager->clearAll();
    delete _chunkManager;
    _chunkManager = NULL;
    GameManager::chunkManager = NULL;
}

void VoxelWorld::updatePhysics(const glm::dvec3 &position, const glm::dvec3 &viewDir) {
    // Update Cellular automata Engine
    globalMultiplePreciseTimer.start("CAEngine Update");
    GameManager::caEngine->update(*_chunkManager);

    // Update physics engine
    globalMultiplePreciseTimer.start("Physics Engine");
    Chunk::modifyLock.lock();
    GameManager::physicsEngine->update(viewDir);
    Chunk::modifyLock.unlock();
}
