#include "stdafx.h"
#include "SphericalVoxelComponent.h"
#include "SphericalTerrainPatch.h"

#include "ChunkManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "SphericalTerrainGenerator.h"
#include "VoxelPlanetMapper.h"

#include <Vorb/IOManager.h>

SphericalVoxelComponent::SphericalVoxelComponent() {
    // Empty
}

SphericalVoxelComponent::~SphericalVoxelComponent() {
    // Empty
}

void SphericalVoxelComponent::init(const SphericalTerrainData* sphericalTerrainData, const IOManager* saveFileIom,
                                   const glm::dvec3 &gpos, vvox::VoxelMapData* startingMapData) {
    m_sphericalTerrainData = sphericalTerrainData;
    m_saveFileIom = saveFileIom;

    // Allocate resources
    m_physicsEngine = new PhysicsEngine();
    m_chunkManager = new ChunkManager(m_physicsEngine);
    m_chunkIo = new ChunkIOManager(saveFileIom->getSearchDirectory());
    m_particleEngine = new ParticleEngine();

    // Init the mapper that will handle spherical voxel mapping
    m_voxelPlanetMapper = new vvox::VoxelPlanetMapper((i32)sphericalTerrainData->getRadius() / CHUNK_WIDTH);

    // Set up the chunk manager
    m_chunkManager->initialize(gpos, m_voxelPlanetMapper,
                               startingMapData,
                               m_chunkIo, 0);
}

void SphericalVoxelComponent::update(const Camera* voxelCamera) {
    m_chunkManager->update(voxelCamera);
}

void SphericalVoxelComponent::getClosestChunks(glm::dvec3 &coord, Chunk **chunks) {

}

void SphericalVoxelComponent::endSession() {
    delete m_physicsEngine;
    m_physicsEngine = nullptr;

    delete m_chunkManager;
    m_chunkManager = nullptr;

    delete m_chunkIo;
    m_chunkIo = nullptr;

    delete m_particleEngine;
    m_particleEngine = nullptr;

    delete m_voxelPlanetMapper;
    m_voxelPlanetMapper = nullptr;
}

void SphericalVoxelComponent::destroyVoxels() {
    m_enabled = false;
}

void SphericalVoxelComponent::updatePhysics(const Camera* camera) {

}
