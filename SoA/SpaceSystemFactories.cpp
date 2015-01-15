#include "stdafx.h"
#include "SpaceSystemFactories.h"

#include "SpaceSystem.h"
#include "ChunkManager.h"
#include "PhysicsEngine.h"
#include "ChunkIOManager.h"
#include "ParticleEngine.h"
#include "VoxelPlanetMapper.h"

vcore::ComponentID SpaceSystemFactories::addSphericalVoxelComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                    vcore::ComponentID sphericalTerrainComponent,
                                                                    const vio::IOManager* saveGameIom) {
#define VOXELS_PER_KM 2000.0
    
    vcore::ComponentID svCmpId = spaceSystem->m_sphericalVoxelCT.add(entity);
    auto& svcmp = spaceSystem->m_sphericalVoxelCT.get(svCmpId);

    auto& stcmp = spaceSystem->m_sphericalTerrainCT.get(sphericalTerrainComponent);

    // Get component handles
    svcmp.axisRotationComponent = stcmp.axisRotationComponent;
    svcmp.namePositionComponent = stcmp.namePositionComponent;
    svcmp.sphericalTerrainComponent = sphericalTerrainComponent;

    svcmp.voxelRadius = stcmp.sphericalTerrainData->getRadius() * VOXELS_PER_KM;

    // TODO(Ben): Destroy these
    svcmp.physicsEngine = new PhysicsEngine();
    svcmp.chunkManager = new ChunkManager(svcmp.physicsEngine);
    svcmp.chunkIo = new ChunkIOManager("TESTSAVEDIR");
    svcmp.particleEngine = new ParticleEngine();
    svcmp.generator = stcmp.generator;
    svcmp.voxelPlanetMapper = new vvox::VoxelPlanetMapper((i32)svcmp.voxelRadius / CHUNK_WIDTH);
    svcmp.planetGenData = stcmp.planetGenData;
    svcmp.sphericalTerrainData = stcmp.sphericalTerrainData;
    svcmp.saveFileIom = saveGameIom;
    

    /*  this->sphericalTerrainData = sphericalTerrainData;
    this->saveFileIom = saveFileIom;

    // Allocate resources
    physicsEngine = new PhysicsEngine();
    chunkManager = new ChunkManager(physicsEngine);
    chunkIo = new ChunkIOManager(saveFileIom->getSearchDirectory().getString());
    particleEngine = new ParticleEngine();
    generator = terrainGenerator;

    // Init the mapper that will handle spherical voxel mapping
    voxelPlanetMapper = new vvox::VoxelPlanetMapper((i32)sphericalTerrainData->getRadius() / CHUNK_WIDTH);

    // Set up the chunk manager
    chunkManager->initialize(gpos, voxelPlanetMapper,
    generator,
    startingMapData,
    chunkIo);*/
    return svCmpId;
}

void SpaceSystemFactories::removeSphericalVoxelComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->m_sphericalVoxelCT.remove(entity);
}
