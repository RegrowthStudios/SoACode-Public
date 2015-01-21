#include "stdafx.h"
#include "SpaceSystemFactories.h"

#include "ChunkIOManager.h"
#include "ChunkManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "VoxelPlanetMapper.h"

#include "SphericalTerrainMeshManager.h"
#include "SpaceSystemFactories.h"

vcore::ComponentID SpaceSystemFactories::addSphericalVoxelComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                    vcore::ComponentID sphericalTerrainComponent,
                                                                    const vvox::VoxelMapData* startingMapData,
                                                                    const f64v3& gridPosition,
                                                                    const SoaState* soaState) {
#define VOXELS_PER_KM 2000.0
    
    vcore::ComponentID svCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, entity);
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
    svcmp.chunkIo = new ChunkIOManager("TESTSAVEDIR"); // TODO(Ben): Fix
    svcmp.chunkManager = new ChunkManager(svcmp.physicsEngine, svcmp.voxelPlanetMapper,
                                          svcmp.generator, startingMapData,
                                          svcmp.chunkIo,
                                          gridPosition, svcmp.voxelRadius);
    svcmp.particleEngine = new ParticleEngine();
    
    svcmp.planetGenData = stcmp.planetGenData;
    svcmp.sphericalTerrainData = stcmp.sphericalTerrainData;
    svcmp.saveFileIom = &soaState->saveFileIom;
    
    return svCmpId;
}

void SpaceSystemFactories::removeSphericalVoxelComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent("SphericalVoxel", entity);
}

vcore::ComponentID SpaceSystemFactories::addAxisRotationComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                  const f64q& axisOrientation, f64 startAngle,
                                                                  f64 angularSpeed) {
    vcore::ComponentID arCmpId = spaceSystem->addComponent("AxisRotation", entity);
    auto& arCmp = spaceSystem->m_axisRotationCT.get(arCmpId);
    arCmp.axisOrientation = axisOrientation;

}

void SpaceSystemFactories::removeAxisRotationComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent("AxisRotation", entity);
}

vcore::ComponentID SpaceSystemFactories::addSphericalTerrainComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                      vcore::ComponentID npComp,
                                                                      vcore::ComponentID arComp,
                                                                      f64 radius, PlanetGenData* planetGenData,
                                                                      vg::GLProgram* normalProgram,
                                                                      vg::TextureRecycler* normalMapRecycler) {
    vcore::ComponentID stCmpId = spaceSystem->addComponent("SphericalTerrain", entity);
    auto& stCmp = spaceSystem->m_sphericalTerrainCT.get(stCmpId);
    
    stCmp.namePositionComponent = npComp;
    stCmp.axisRotationComponent = arComp;
    stCmp.planetGenData = planetGenData;

    stCmp.meshManager = new SphericalTerrainMeshManager(planetGenData,
                                                  normalMapRecycler);
    stCmp.generator = new SphericalTerrainGenerator(radius, stCmp.meshManager,
                                              planetGenData,
                                              normalProgram, normalMapRecycler);
    stCmp.rpcDispatcher = new TerrainRpcDispatcher(stCmp.generator);

    f64 patchWidth = (radius * 2.000) / PATCH_ROW;
    stCmp.sphericalTerrainData = new SphericalTerrainData(radius, patchWidth);
}

void SpaceSystemFactories::removeSphericalTerrainComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent("SphericalTerrain", entity);
}
