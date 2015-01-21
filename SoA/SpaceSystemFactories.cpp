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

vcore::ComponentID makeOrbitFromProps(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                        const SystemBodyKegProperties* sysProps) {
    
    f64q orientation;
    // Calculate orbit orientation from normal
    const f64v3 right(1.0, 0.0, 0.0);
    if (right == -sysProps->orbitNormal) {
        f64v3 eulers(0.0, M_PI, 0.0);
        orientation = f64q(eulers);
    } else if (right != sysProps->orbitNormal) {
        orientation = quatBetweenVectors(right, sysProps->orbitNormal);
    }

    return SpaceSystemFactories::addOrbitComponent(spaceSystem, entity, sysProps->eccentricity,
                                                   sysProps->period, sysProps->pathColor, orientation);
}

vcore::EntityID SpaceSystemFactories::createPlanet(OUT SpaceSystem* spaceSystem,
                                    const SystemBodyKegProperties* sysProps,
                                    const PlanetKegProperties* properties,
                                    SystemBody* body) {
    body->entity = spaceSystem->addEntity();
    const vcore::EntityID& id = body->entity;

    const f64v3 up(0.0, 1.0, 0.0);
    vcore::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, quatBetweenVectors(up, glm::normalize(properties->axis)),
                             0.0, properties->angularSpeed);

    f64v3 tmpPos(0.0);
    vcore::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    addSphericalTerrainComponent(spaceSystem, id, npCmp, arCmp, properties->diameter / 2.0,
                                 properties->planetGenData,
                                 spaceSystem->glProgramManager->getProgram("NormalMapGen"),
                                 spaceSystem->normalMapRecycler);

    addSphericalGravityComponent(spaceSystem, id, properties->diameter / 2.0, properties->mass);

    makeOrbitFromProps(spaceSystem, id, sysProps);
    return id;
}

void SpaceSystemFactories::destroyPlanet(OUT SpaceSystem* gameSystem, vcore::EntityID planetEntity) {
    // TODO: implement
}

vcore::EntityID SpaceSystemFactories::createStar(OUT SpaceSystem* spaceSystem,
                                  const SystemBodyKegProperties* sysProps,
                                  const PlanetKegProperties* properties,
                                  SystemBody* body) {
    body->entity = spaceSystem->addEntity();
    const vcore::EntityID& id = body->entity;

    const f64v3 up(0.0, 1.0, 0.0);
    vcore::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, quatBetweenVectors(up, glm::normalize(properties->axis)),
                                                        0.0, properties->angularSpeed);

    f64v3 tmpPos(0.0);
    vcore::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    addSphericalGravityComponent(spaceSystem, id, properties->diameter / 2.0, properties->mass);

    makeOrbitFromProps(spaceSystem, id, sysProps);

    return id;
}

void destroyStar(OUT SpaceSystem* gameSystem, vcore::EntityID planetEntity) {
    // TODO: implement
}

/// GasGiant entity
vcore::EntityID SpaceSystemFactories::createGasGiant(OUT SpaceSystem* spaceSystem,
                                      const SystemBodyKegProperties* sysProps,
                                      const PlanetKegProperties* properties,
                                      SystemBody* body) {
    body->entity = spaceSystem->addEntity();
    const vcore::EntityID& id = body->entity;

    const f64v3 up(0.0, 1.0, 0.0);
    vcore::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, quatBetweenVectors(up, glm::normalize(properties->axis)),
                                                        0.0, properties->angularSpeed);

    f64v3 tmpPos(0.0);
    vcore::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    addSphericalGravityComponent(spaceSystem, id, properties->diameter / 2.0, properties->mass);

    makeOrbitFromProps(spaceSystem, id, sysProps);

    return id;
}

void destroyGasGiant(OUT SpaceSystem* gameSystem, vcore::EntityID planetEntity) {
    // TODO: implement
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
    return arCmpId;
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
    return stCmpId;
}

void SpaceSystemFactories::removeSphericalTerrainComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent("SphericalTerrain", entity);
}

vcore::ComponentID SpaceSystemFactories::addSphericalGravityComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                      f64 radius, f64 mass) {
    vcore::ComponentID sgCmpId = spaceSystem->addComponent("SphericalTerrain", entity);
    auto& sgCmp = spaceSystem->m_sphericalGravityCT.get(sgCmpId);
    sgCmp.radius = radius;
    sgCmp.mass = mass;
    return sgCmpId;
}

void SpaceSystemFactories::removeSphericalGravityComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent("SphericalGravity", entity);
}

vcore::ComponentID SpaceSystemFactories::addNamePositionComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                  const nString& name, const f64v3& position) {
    vcore::ComponentID npCmpId = spaceSystem->addComponent("NamePosition", entity);
    auto& npCmp = spaceSystem->m_namePositionCT.get(npCmpId);
    npCmp.name = name;
    npCmp.position = position;
    return npCmpId;
}

void SpaceSystemFactories::removeNamePositionComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent("NamePosition", entity);
}

vcore::ComponentID SpaceSystemFactories::addOrbitComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                           f64 eccentricity, f64 orbitalPeriod,
                                                           const ui8v4& pathColor, const f64q& orientation) {
    vcore::ComponentID oCmpId = spaceSystem->addComponent("Orbit", entity);
    auto& oCmp = spaceSystem->m_orbitCT.get(oCmpId);
    oCmp.eccentricity = eccentricity;
    oCmp.orbitalPeriod = orbitalPeriod;
    oCmp.pathColor = pathColor;
    oCmp.orientation = orientation;
    return oCmpId;
}

void SpaceSystemFactories::removeOrbitComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent("Orbit", entity);
}
