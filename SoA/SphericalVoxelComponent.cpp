#include "stdafx.h"
#include "SphericalVoxelComponent.h"
#include "SphericalTerrainPatch.h"

#include "ChunkManager.h"
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
    m_chunkManager = std::make_unique<ChunkManager>();
    m_chunkIo = std::make_unique<ChunkIOManager>();
    m_physicsEngine = std::make_unique<PhysicsEngine>();
    m_particleEngine = std::make_unique<ParticleEngine>();
    m_generator = std::make_unique<SphericalTerrainGenerator>();

    // Init the mapper that will handle spherical voxel mapping
    m_voxelPlanetMapper = std::make_unique<vvox::VoxelPlanetMapper>((i32)sphericalTerrainData->getRadius() / CHUNK_WIDTH);

    // Set up the chunk manager
    m_chunkManager->initialize(gpos, m_voxelPlanetMapper.get(),
                               startingMapData,
                               m_chunkIo.get(), 0);
}

void SphericalVoxelComponent::update(const Camera* voxelCamera) {
    m_chunkManager->update(voxelCamera);
}

void SphericalVoxelComponent::getClosestChunks(glm::dvec3 &coord, Chunk **chunks) {

}

void SphericalVoxelComponent::endSession() {

}

void SphericalVoxelComponent::destroyVoxels() {
    m_enabled = false;
}

void SphericalVoxelComponent::updatePhysics(const Camera* camera) {

}
