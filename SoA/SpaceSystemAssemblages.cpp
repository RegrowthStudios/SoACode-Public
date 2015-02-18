#include "stdafx.h"
#include "SpaceSystemAssemblages.h"

#include "ChunkIOManager.h"
#include "ChunkManager.h"
#include "FarTerrainPatch.h"
#include "ParticleEngine.h"
#include "PhysicsEngine.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "SphericalTerrainComponentUpdater.h"
#include "SphericalTerrainCpuGenerator.h"
#include "SphericalTerrainGpuGenerator.h"

#include "TerrainPatchMeshManager.h"
#include "SpaceSystemAssemblages.h"
#include "SpaceSystemLoadStructs.h"

// TEMPORARY
#include "GameManager.h"
#include "TexturePackLoader.h"
#include "PlanetData.h"

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

    return SpaceSystemAssemblages::addOrbitComponent(spaceSystem, entity, sysProps->eccentricity,
                                                   sysProps->period, sysProps->pathColor, orientation);
}

vcore::EntityID SpaceSystemAssemblages::createPlanet(OUT SpaceSystem* spaceSystem,
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

    addSphericalTerrainComponent(spaceSystem, id, npCmp, arCmp,
                                 properties->planetGenData,
                                 spaceSystem->normalMapGenProgram.get(),
                                 spaceSystem->normalMapRecycler.get());

    addSphericalGravityComponent(spaceSystem, id, properties->diameter / 2.0, properties->mass);

    makeOrbitFromProps(spaceSystem, id, sysProps);
    return id;
}

void SpaceSystemAssemblages::destroyPlanet(OUT SpaceSystem* gameSystem, vcore::EntityID planetEntity) {
    // TODO: implement
}

vcore::EntityID SpaceSystemAssemblages::createStar(OUT SpaceSystem* spaceSystem,
                                  const SystemBodyKegProperties* sysProps,
                                  const StarKegProperties* properties,
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
vcore::EntityID SpaceSystemAssemblages::createGasGiant(OUT SpaceSystem* spaceSystem,
                                      const SystemBodyKegProperties* sysProps,
                                      const GasGiantKegProperties* properties,
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

vcore::ComponentID SpaceSystemAssemblages::addSphericalVoxelComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                      vcore::ComponentID sphericalTerrainComponent,
                                                                      vcore::ComponentID farTerrainComponent,
                                                                      vcore::ComponentID axisRotationComponent,
                                                                      vcore::ComponentID namePositionComponent,
                                                                      const VoxelPosition3D& startVoxelPos,
                                                                      const SoaState* soaState) {
#define VOXELS_PER_KM 2000.0
    
    vcore::ComponentID svCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, entity);
    auto& svcmp = spaceSystem->m_sphericalVoxelCT.get(svCmpId);

    auto& ftcmp = spaceSystem->m_farTerrainCT.get(farTerrainComponent);

    // Get component handles
    svcmp.sphericalTerrainComponent = sphericalTerrainComponent;
    svcmp.axisRotationComponent = axisRotationComponent;
    svcmp.namePositionComponent = namePositionComponent;
    svcmp.farTerrainComponent = farTerrainComponent;

    svcmp.voxelRadius = ftcmp.sphericalTerrainData->radius * VOXELS_PER_KM;

    svcmp.physicsEngine = new PhysicsEngine();

    svcmp.generator = ftcmp.gpuGenerator;
    svcmp.chunkIo = new ChunkIOManager("TESTSAVEDIR"); // TODO(Ben): Fix
    svcmp.chunkManager = new ChunkManager(svcmp.physicsEngine,
                                          svcmp.generator, startVoxelPos,
                                          svcmp.chunkIo,
                                          ftcmp.sphericalTerrainData->radius * 2000.0);
    svcmp.particleEngine = new ParticleEngine();
    
    svcmp.planetGenData = ftcmp.planetGenData;
    svcmp.sphericalTerrainData = ftcmp.sphericalTerrainData;
    svcmp.saveFileIom = &soaState->saveFileIom;
    
    // TODO(Ben): This isn't a good place for this
    ColorRGB8* cmap = GameManager::texturePackLoader->getColorMap("biome");
    ui32 index = GameManager::texturePackLoader->getColorMapIndex("biome");
    glBindTexture(GL_TEXTURE_2D, ftcmp.planetGenData->terrainColorMap.id);
    if (ftcmp.planetGenData->terrainColorMap.width != 256 ||
        ftcmp.planetGenData->terrainColorMap.height != 256) {
        std::cerr << "Terrain color map needs to be 256x256";
    }
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, cmap);
    glBindTexture(GL_TEXTURE_2D, 0);

    // BEWARE HACKY CRAP
    for (int i = 0; i < Blocks.size(); i++) {
        if (Blocks[i].pxTexInfo.base.useMapColor == "biome") {
            Blocks[i].pxTexInfo.base.colorMapIndex = index;
        }
        if (Blocks[i].pxTexInfo.overlay.useMapColor == "biome") {
            Blocks[i].pxTexInfo.overlay.colorMapIndex = index;
        }
        if (Blocks[i].pyTexInfo.base.useMapColor == "biome") {
            Blocks[i].pyTexInfo.base.colorMapIndex = index;
        }
        if (Blocks[i].pyTexInfo.overlay.useMapColor == "biome") {
            Blocks[i].pyTexInfo.overlay.colorMapIndex = index;
        }
        if (Blocks[i].pzTexInfo.base.useMapColor == "biome") {
            Blocks[i].pzTexInfo.base.colorMapIndex = index;
        }
        if (Blocks[i].pzTexInfo.overlay.useMapColor == "biome") {
            Blocks[i].pzTexInfo.overlay.colorMapIndex = index;
        }
    }

    return svCmpId;
}

void SpaceSystemAssemblages::removeSphericalVoxelComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, entity);
}

vcore::ComponentID SpaceSystemAssemblages::addAxisRotationComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                  const f64q& axisOrientation, f64 startAngle,
                                                                  f64 angularSpeed) {
    vcore::ComponentID arCmpId = spaceSystem->addComponent("AxisRotation", entity);
    auto& arCmp = spaceSystem->m_axisRotationCT.get(arCmpId);
    arCmp.axisOrientation = axisOrientation;
    arCmp.currentRotation = startAngle;
    arCmp.angularSpeed_RS = angularSpeed;
    return arCmpId;
}

void SpaceSystemAssemblages::removeAxisRotationComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent("AxisRotation", entity);
}

vcore::ComponentID SpaceSystemAssemblages::addSphericalTerrainComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                      vcore::ComponentID npComp,
                                                                      vcore::ComponentID arComp,
                                                                      PlanetGenData* planetGenData,
                                                                      vg::GLProgram* normalProgram,
                                                                      vg::TextureRecycler* normalMapRecycler) {
    vcore::ComponentID stCmpId = spaceSystem->addComponent("SphericalTerrain", entity);
    auto& stCmp = spaceSystem->m_sphericalTerrainCT.get(stCmpId);
    
    stCmp.namePositionComponent = npComp;
    stCmp.axisRotationComponent = arComp;
    stCmp.planetGenData = planetGenData;

    stCmp.meshManager = new TerrainPatchMeshManager(planetGenData,
                                                  normalMapRecycler);
    stCmp.gpuGenerator = new SphericalTerrainGpuGenerator(stCmp.meshManager,
                                              planetGenData,
                                              normalProgram, normalMapRecycler);
    stCmp.cpuGenerator = new SphericalTerrainCpuGenerator(stCmp.meshManager,
                                                       planetGenData);
    stCmp.rpcDispatcher = new TerrainRpcDispatcher(stCmp.gpuGenerator, stCmp.cpuGenerator);

    f64 patchWidth = (planetGenData->radius * 2.000) / ST_PATCH_ROW;
    stCmp.sphericalTerrainData = new TerrainPatchData(planetGenData->radius, patchWidth);

    return stCmpId;
}

void SpaceSystemAssemblages::removeSphericalTerrainComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    auto& stcmp = spaceSystem->m_sphericalTerrainCT.getFromEntity(entity);

    delete stcmp.meshManager;
    delete stcmp.gpuGenerator;
    delete stcmp.cpuGenerator;
    delete stcmp.rpcDispatcher;
    delete stcmp.sphericalTerrainData;
    spaceSystem->deleteComponent("SphericalTerrain", entity);
}

/// Spherical terrain component
vcore::ComponentID SpaceSystemAssemblages::addFarTerrainComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                  SphericalTerrainComponent& parentCmp,
                                                                  WorldCubeFace face) {
    vcore::ComponentID ftCmpId = spaceSystem->addComponent("FarTerrain", entity);
    auto& ftCmp = spaceSystem->m_farTerrainCT.get(ftCmpId);

    ftCmp.planetGenData = parentCmp.planetGenData;
    ftCmp.meshManager = parentCmp.meshManager;
    ftCmp.gpuGenerator = parentCmp.gpuGenerator;
    ftCmp.cpuGenerator = parentCmp.cpuGenerator;
    ftCmp.rpcDispatcher = parentCmp.rpcDispatcher;
    ftCmp.sphericalTerrainData = parentCmp.sphericalTerrainData;

    ftCmp.face = face;

    return ftCmpId;
}

void SpaceSystemAssemblages::removeFarTerrainComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    auto& ftcmp = spaceSystem->m_farTerrainCT.getFromEntity(entity);

    if (ftcmp.patches) delete[] ftcmp.patches;
    ftcmp.patches = nullptr;
    ftcmp.gpuGenerator = nullptr;
    spaceSystem->deleteComponent("FarTerrain", entity);
}

vcore::ComponentID SpaceSystemAssemblages::addSphericalGravityComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                      f64 radius, f64 mass) {
    vcore::ComponentID sgCmpId = spaceSystem->addComponent("SphericalGravity", entity);
    auto& sgCmp = spaceSystem->m_sphericalGravityCT.get(sgCmpId);
    sgCmp.radius = radius;
    sgCmp.mass = mass;
    return sgCmpId;
}

void SpaceSystemAssemblages::removeSphericalGravityComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent("SphericalGravity", entity);
}

vcore::ComponentID SpaceSystemAssemblages::addNamePositionComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
                                                                  const nString& name, const f64v3& position) {
    vcore::ComponentID npCmpId = spaceSystem->addComponent("NamePosition", entity);
    auto& npCmp = spaceSystem->m_namePositionCT.get(npCmpId);
    npCmp.name = name;
    npCmp.position = position;
    return npCmpId;
}

void SpaceSystemAssemblages::removeNamePositionComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent("NamePosition", entity);
}

vcore::ComponentID SpaceSystemAssemblages::addOrbitComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity,
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

void SpaceSystemAssemblages::removeOrbitComponent(OUT SpaceSystem* spaceSystem, vcore::EntityID entity) {
    spaceSystem->deleteComponent("Orbit", entity);
}
