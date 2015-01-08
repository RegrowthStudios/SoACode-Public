#include "stdafx.h"
#include "SphericalVoxelComponent.h"
#include "SphericalTerrainPatch.h"

#include "ChunkManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "SphericalTerrainGenerator.h"
#include "VoxelPlanetMapper.h"

#include <Vorb/IOManager.h>

void SphericalVoxelComponent::init(const SphericalTerrainData* sphericalTerrainData, const IOManager* saveFileIom,
                                   SphericalTerrainGenerator* terrainGenerator,
                                   const glm::dvec3 &gpos, vvox::VoxelMapData* startingMapData) {
    m_sphericalTerrainData = sphericalTerrainData;
    m_saveFileIom = saveFileIom;

    // Allocate resources
    m_physicsEngine = new PhysicsEngine();
    m_chunkManager = new ChunkManager(m_physicsEngine);
    m_chunkIo = new ChunkIOManager(saveFileIom->getSearchDirectory());
    m_particleEngine = new ParticleEngine();
    m_generator = terrainGenerator;

    // Init the mapper that will handle spherical voxel mapping
    m_voxelPlanetMapper = new vvox::VoxelPlanetMapper((i32)sphericalTerrainData->getRadius() / CHUNK_WIDTH);

    // Set up the chunk manager
    m_chunkManager->initialize(gpos, m_voxelPlanetMapper,
                               m_generator,
                               startingMapData,
                               m_chunkIo);
}