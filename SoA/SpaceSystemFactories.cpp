#include "stdafx.h"
#include "SpaceSystemFactories.h"

#include "ChunkIOManager.h"
#include "ChunkManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "VoxelPlanetMapper.h"

vcore::ComponentID SpaceSystemFactories::addSphericalVoxelComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                    vcore::ComponentID sphericalTerrainComponent,
                                                                    const vvox::VoxelMapData* startingMapData,
                                                                    const f64v3& gridPosition,
                                                                    const SoaState* soaState) {
#define VOXELS_PER_KM 2000.0
    
    vcore::ComponentID svCmpId = spaceSystem->addComponent("Spherical Voxel", entity);
    auto& svcmp = spaceSystem->m_sphericalVoxelCT.get(svCmpId);

    auto& stcmp = spaceSystem->m_sphericalTerrainCT.get(sphericalTerrainComponent);

    // Get component handles
    svcmp.axisRotationComponent = stcmp.axisRotationComponent;
    svcmp.namePositionComponent = stcmp.namePositionComponent;
    svcmp.sphericalTerrainComponent = sphericalTerrainComponent;

    svcmp.voxelRadius = stcmp.sphericalTerrainData->getRadius() * VOXELS_PER_KM;

    svcmp.physicsEngine = new PhysicsEngine();
    svcmp.voxelPlanetMapper = new vvox::VoxelPlanetMapper((i32)svcmp.voxelRadius / CHUNK_WIDTH);
    svcmp.generator = stcmp.generator;
    svcmp.chunkIo = new ChunkIOManager("TESTSAVEDIR");
    svcmp.chunkManager = new ChunkManager(svcmp.physicsEngine, svcmp.voxelPlanetMapper,
                                          svcmp.generator, startingMapData,
                                          svcmp.chunkIo,
                                          gridPosition, svcmp.voxelRadius);
    svcmp.particleEngine = new ParticleEngine();
    
    svcmp.planetGenData = stcmp.planetGenData;
    svcmp.sphericalTerrainData = stcmp.sphericalTerrainData;
    svcmp.saveFileIom = &soaState->saveFileIom;
    

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
    spaceSystem->deleteComponent("SphericalVoxel", entity);
}
