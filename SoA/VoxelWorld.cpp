#include "stdafx.h"
#include "VoxelWorld.h"

#include "VoxelPlanetMapper.h"

#include "BlockData.h"
#include "CAEngine.h"
#include "Camera.h"
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


void VoxelWorld::initialize(const glm::dvec3 &gpos, vvox::VoxelMapData* startingMapData, Planet *planet, GLuint flags)
{
    if (_chunkManager) {
        pError("VoxelWorld::initialize() called twice before end session!");
        delete _chunkManager;
    }
    _chunkManager = new ChunkManager();
    GameManager::chunkManager = _chunkManager;
    
    if (planet == NULL) showMessage("Initialized chunk manager with NULL planet!");

    _chunkManager->planet = planet;

    vvox::VoxelPlanetMapper* voxelPlanetMapper = new vvox::VoxelPlanetMapper(planet->facecsGridWidth);
    _chunkManager->initialize(gpos, voxelPlanetMapper, startingMapData, flags);

    setPlanet(planet);
}

void VoxelWorld::update(const Camera* camera)
{
    // Update the Chunks
    _chunkManager->update(camera);
    // Update the physics
    updatePhysics(camera);
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
    _chunkManager->destroy();
    delete _chunkManager;
    _chunkManager = NULL;
    GameManager::chunkManager = NULL;
}

void VoxelWorld::updatePhysics(const Camera* camera) {

    GameManager::chunkManager->updateCaPhysics();

    // Update physics engine
    globalMultiplePreciseTimer.start("Physics Engine");

    GameManager::physicsEngine->update(f64v3(camera->getDirection()));
}
