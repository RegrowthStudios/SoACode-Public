#include "stdafx.h"
#include "VoxelWorld.h"

#include "VoxelPlanetMapper.h"

#include "BlockData.h"
#include "Chunk.h"
#include "ChunkManager.h"
#include "Errors.h"
#include "GameManager.h"
#include "Item.h"
#include "PhysicsEngine.h"
#include "Planet.h"
#include "Sound.h"
#include "global.h"
#include "utils.h"
#include "VoxelEditor.h"

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

    initializeThreadPool();
    
    if (planet == NULL) showMessage("Initialized chunkmanager with NULL planet!");

    _chunkManager->planet = planet;

    vvoxel::VoxelPlanetMapper* voxelPlanetMapper = new vvoxel::VoxelPlanetMapper();
    _chunkManager->initialize(gpos, voxelPlanetMapper, startingMapData, flags);

    setPlanet(planet);
}

void VoxelWorld::update(const glm::dvec3 &position, const glm::dvec3 &viewDir)
{
    _chunkManager->update(position, viewDir);
}

void VoxelWorld::initializeThreadPool()
{
    size_t hc = thread::hardware_concurrency();
    if (hc > 1) hc--;
    if (hc > 1) hc--;
    SDL_GL_MakeCurrent(mainWindow, NULL);
    _chunkManager->threadPool.initialize(hc);
    SDL_Delay(100);
    mainContextLock.lock();
    SDL_GL_MakeCurrent(mainWindow, mainOpenGLContext);
    mainContextLock.unlock();
}

void VoxelWorld::closeThreadPool()
{
    _chunkManager->threadPool.close();
}

void VoxelWorld::setPlanet(Planet *planet)
{
    _planet = planet;
    GameManager::planet = planet;
}

int VoxelWorld::getClosestChunks(glm::dvec3 &coord, class Chunk **chunks)
{
    return _chunkManager->getClosestChunks(coord, chunks);
}

void VoxelWorld::endSession()
{
    GameManager::chunkIOManager->onQuit();
    _chunkManager->clearAll();
    delete _chunkManager;
    _chunkManager = NULL;
    GameManager::chunkManager = NULL;
}