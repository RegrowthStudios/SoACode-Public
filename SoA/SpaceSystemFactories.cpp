#include "stdafx.h"
#include "SpaceSystemFactories.h"

#include "ChunkIOManager.h"
#include "ChunkManager.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "SphericalTerrainComponentUpdater.h"
#include "SphericalTerrainGenerator.h"
#include "VoxelPlanetMapper.h"

#include "SphericalTerrainMeshManager.h"
#include "SpaceSystemFactories.h"
#include "SpaceSystemLoadStructs.h"

void setOrbitProperties(OUT SpaceSystem* spaceSystem, vcore::ComponentID cmp, const SystemBodyKegProperties* sysProps) {
    // Set basic properties
    auto& orbitCmp = spaceSystem->m_orbitCT.get(cmp);
    orbitCmp.eccentricity = sysProps->eccentricity;
    orbitCmp.orbitalPeriod = sysProps->period;
    orbitCmp.pathColor = sysProps->pathColor;

    // Calculate orbit orientation from normal
    const f64v3 right(1.0, 0.0, 0.0);
    if (right == -sysProps->orbitNormal) {
        f64v3 eulers(0.0, M_PI, 0.0);
        orbitCmp.orientation = f64q(eulers);
    } else if (right != sysProps->orbitNormal) {
        orbitCmp.orientation = quatBetweenVectors(right, sysProps->orbitNormal);
    }
}

vcore::EntityID SpaceSystemFactories::createPlanet(OUT SpaceSystem* spaceSystem,
                                    const SystemBodyKegProperties* sysProps,
                                    const PlanetKegProperties* properties,
                                    SystemBody* body) {
    body->entity = spaceSystem->addEntity();
    const vcore::EntityID& id = body->entity;

    const f64v3 up(0.0, 1.0, 0.0);
    addAxisRotationComponent(spaceSystem, id, quatBetweenVectors(up, glm::normalize(properties->axis)),
                             0.0, properties->angularSpeed);
    vcore::ComponentID npCmp = spaceSystem->addComponent(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, id);
    vcore::ComponentID oCmp = spaceSystem->addComponent(SPACE_SYSTEM_CT_ORBIT_NAME, id);
    addSphericalTerrainComponent(spaceSystem, id, npCmp, arCmp, properties->diameter / 2.0,
                                 properties->planetGenData,
                                 spaceSystem->glProgramManager->getProgram("NormalMapGen"),
                                 spaceSystem->normalMapRecycler)
    vcore::ComponentID sgCmp = spaceSystem->addComponent(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, id);

    m_sphericalGravityCT.get(sgCmp).init(properties->diameter / 2.0,
                                         properties->mass);


    // Set the name
    m_namePositionCT.get(npCmp).name = body->name;

    setOrbitProperties(oCmp, sysProps);
}

void SpaceSystemFactories::destroyPlanet(OUT SpaceSystem* gameSystem, vcore::EntityID planetEntity) {

}

vcore::EntityID createStar(OUT SpaceSystem* spaceSystem,
                                  const SystemBodyKegProperties* sysProps,
                                  const PlanetKegProperties* properties,
                                  SystemBody* body) {
    body->entity = new vcore::Entity(addEntity());
    const vcore::EntityID& id = body->entity->id;

    vcore::ComponentID npCmp = addComponent(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, id);
    vcore::ComponentID arCmp = addComponent(SPACE_SYSTEM_CT_AXISROTATION_NAME, id);
    vcore::ComponentID oCmp = addComponent(SPACE_SYSTEM_CT_ORBIT_NAME, id);
    vcore::ComponentID sgCmp = addComponent(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, id);

    f64v3 up(0.0, 1.0, 0.0);
    m_axisRotationCT.get(arCmp).init(properties->angularSpeed,
                                     0.0,
                                     quatBetweenVectors(up, glm::normalize(properties->axis)));

    m_sphericalGravityCT.get(sgCmp).init(properties->diameter / 2.0,
                                         properties->mass);

    // Set the name
    m_namePositionCT.get(npCmp).name = body->name;

    setOrbitProperties(oCmp, sysProps);
}

void destroyStar(OUT SpaceSystem* gameSystem, vcore::EntityID planetEntity) {

}

/// GasGiant entity
vcore::EntityID createGasGiant(OUT SpaceSystem* spaceSystem,
                                      const SystemBodyKegProperties* sysProps,
                                      const PlanetKegProperties* properties,
                                      SystemBody* body) {
    body->entity = new vcore::Entity(addEntity());
    const vcore::EntityID& id = body->entity->id;

    vcore::ComponentID npCmp = addComponent(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, id);
    vcore::ComponentID arCmp = addComponent(SPACE_SYSTEM_CT_AXISROTATION_NAME, id);
    vcore::ComponentID oCmp = addComponent(SPACE_SYSTEM_CT_ORBIT_NAME, id);
    vcore::ComponentID sgCmp = addComponent(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, id);

    f64v3 up(0.0, 1.0, 0.0);
    m_axisRotationCT.get(arCmp).init(properties->angularSpeed,
                                     0.0,
                                     quatBetweenVectors(up, glm::normalize(properties->axis)));

    m_sphericalGravityCT.get(sgCmp).init(properties->diameter / 2.0,
                                         properties->mass);

    // Set the name
    m_namePositionCT.get(npCmp).name = body->name;

    setOrbitProperties(oCmp, sysProps);
}

void destroyGasGiant(OUT SpaceSystem* gameSystem, vcore::EntityID planetEntity) {

}

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
