#include "stdafx.h"
#include "VoxelWorld.h"

#include <Vorb/utils.h>

#include "BlockData.h"
#include "CAEngine.h"
#include "Camera.h"
#include "Chunk.h"
#include "ChunkManager.h"
#include "Errors.h"
#include "GameManager.h"
#include "Item.h"
#include "PhysicsEngine.h"
#include "Sound.h"
#include "VoxelEditor.h"
#include "global.h"
#include "VoxelPlanetMapper.h"

VoxelWorld::VoxelWorld() {

}


VoxelWorld::~VoxelWorld()
{
}


void VoxelWorld::initialize(const glm::dvec3 &gpos, vvox::VoxelMapData* startingMapData, GLuint flags)
{
    if (m_chunkManager) {
        pError("VoxelWorld::initialize() called twice before end session!");
        delete m_chunkManager;
    }

    m_physicsEngine = new PhysicsEngine;
    m_chunkManager = new ChunkManager;
    m_chunkIo = new ChunkIOManager;

  /*  
    
    if (planet == NULL) showMessage("Initialized chunk manager with NULL planet!");

    _chunkManager->planet = planet;

    vvox::VoxelPlanetMapper* voxelPlanetMapper = new vvox::VoxelPlanetMapper(planet->facecsGridWidth);
    _chunkManager->initialize(gpos, voxelPlanetMapper, startingMapData, flags);

    setPlanet(planet);*/

}

void VoxelWorld::update(const Camera* camera)
{
    // Update the Chunks
    m_chunkManager->update(camera);
    // Update the physics
    updatePhysics(camera);
}

void VoxelWorld::getClosestChunks(glm::dvec3 &coord, class Chunk **chunks)
{
    m_chunkManager->getClosestChunks(coord, chunks);
}

void VoxelWorld::endSession()
{
    m_chunkIo->onQuit();
    delete m_chunkIo;
    m_chunkManager->destroy();
    delete m_chunkManager;
    m_chunkManager = nullptr;
    delete m_physicsEngine;
    m_physicsEngine = nullptr;
}

void VoxelWorld::updatePhysics(const Camera* camera) {

    m_chunkManager->updateCaPhysics();

    // Update physics engine
    globalMultiplePreciseTimer.start("Physics Engine");

    m_physicsEngine->update(f64v3(camera->getDirection()));
}
