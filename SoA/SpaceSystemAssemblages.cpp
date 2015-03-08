#include "stdafx.h"
#include "SpaceSystemAssemblages.h"

#include "ChunkGrid.h"
#include "ChunkIOManager.h"
#include "ChunkListManager.h"
#include "ChunkMemoryManager.h"
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

#define SEC_PER_DAY 86400.0

vecs::ComponentID makeOrbitFromProps(OUT SpaceSystem* spaceSystem, vecs::EntityID entity,
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

vecs::EntityID SpaceSystemAssemblages::createPlanet(SpaceSystem* spaceSystem,
                                    const SystemBodyKegProperties* sysProps,
                                    const PlanetKegProperties* properties,
                                    SystemBody* body) {
    body->entity = spaceSystem->addEntity();
    const vecs::EntityID& id = body->entity;

    const f64v3 up(0.0, 1.0, 0.0);
    vecs::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, quatBetweenVectors(up, glm::normalize(properties->axis)),
                                                        0.0, properties->rotationalPeriod * SEC_PER_DAY);

    f64v3 tmpPos(0.0);
    vecs::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    addSphericalTerrainComponent(spaceSystem, id, npCmp, arCmp,
                                 properties->planetGenData,
                                 spaceSystem->normalMapGenProgram.get(),
                                 spaceSystem->normalMapRecycler.get());

    addSphericalGravityComponent(spaceSystem, id, npCmp, properties->diameter / 2.0, properties->mass);

    addAtmosphereComponent(spaceSystem, id, npCmp, (properties->diameter / 2.0) * 1.1);

    makeOrbitFromProps(spaceSystem, id, sysProps);
    return id;
}

void SpaceSystemAssemblages::destroyPlanet(SpaceSystem* gameSystem, vecs::EntityID planetEntity) {
    // TODO: implement
}

vecs::EntityID SpaceSystemAssemblages::createStar(SpaceSystem* spaceSystem,
                                  const SystemBodyKegProperties* sysProps,
                                  const StarKegProperties* properties,
                                  SystemBody* body) {
    body->entity = spaceSystem->addEntity();
    const vecs::EntityID& id = body->entity;

    const f64v3 up(0.0, 1.0, 0.0);
    vecs::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, quatBetweenVectors(up, glm::normalize(properties->axis)),
                                                        0.0, properties->rotationalPeriod * SEC_PER_DAY);

    f64v3 tmpPos(0.0);
    vecs::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    addSphericalGravityComponent(spaceSystem, id, npCmp, properties->diameter / 2.0, properties->mass);

    makeOrbitFromProps(spaceSystem, id, sysProps);

    return id;
}

void SpaceSystemAssemblages::destroyStar(SpaceSystem* gameSystem, vecs::EntityID planetEntity) {
    // TODO: implement
}

/// GasGiant entity
vecs::EntityID SpaceSystemAssemblages::createGasGiant(SpaceSystem* spaceSystem,
                                      const SystemBodyKegProperties* sysProps,
                                      const GasGiantKegProperties* properties,
                                      SystemBody* body) {
    body->entity = spaceSystem->addEntity();
    const vecs::EntityID& id = body->entity;

    const f64v3 up(0.0, 1.0, 0.0);
    vecs::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, quatBetweenVectors(up, glm::normalize(properties->axis)),
                                                        0.0, properties->rotationalPeriod * SEC_PER_DAY);

    f64v3 tmpPos(0.0);
    vecs::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    addSphericalGravityComponent(spaceSystem, id, npCmp, properties->diameter / 2.0, properties->mass);

    makeOrbitFromProps(spaceSystem, id, sysProps);

    return id;
}

void SpaceSystemAssemblages::destroyGasGiant(SpaceSystem* gameSystem, vecs::EntityID planetEntity) {
    // TODO: implement
}

vecs::ComponentID SpaceSystemAssemblages::addAtmosphereComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                vecs::ComponentID namePositionComponent, f64 radius) {
    vecs::ComponentID aCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_ATMOSPHERE_NAME, entity);
    auto& aCmp = spaceSystem->m_atmosphereCT.get(aCmpId);
    aCmp.namePositionComponent = namePositionComponent;
    aCmp.radius = radius;
    return aCmpId;
}

void SpaceSystemAssemblages::removeAtmosphereComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_ATMOSPHERE_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addSphericalVoxelComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                      vecs::ComponentID sphericalTerrainComponent,
                                                                      vecs::ComponentID farTerrainComponent,
                                                                      vecs::ComponentID axisRotationComponent,
                                                                      vecs::ComponentID namePositionComponent,
                                                                      WorldCubeFace worldFace,
                                                                      const SoaState* soaState) {

    vecs::ComponentID svCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, entity);
    if (svCmpId == 0) {
        return 0;
    }
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
    svcmp.chunkGrid = new ChunkGrid(worldFace);
    svcmp.chunkListManager = new ChunkListManager();
    svcmp.chunkMemoryManager = new ChunkMemoryManager();
    svcmp.chunkMeshManager = soaState->chunkMeshManager.get();

    // Set up threadpool
    // Check the hardware concurrency
    size_t hc = std::thread::hardware_concurrency();
    // Remove two threads for the render thread and main thread
    if (hc > 1) hc--;
    if (hc > 1) hc--;

    // Initialize the threadpool with hc threads
    svcmp.threadPool = new vcore::ThreadPool<WorkerData>(); 
    svcmp.threadPool->init(hc);
    svcmp.chunkIo->beginThread();
    // Give some time for the threads to spin up
    SDL_Delay(100);

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

void SpaceSystemAssemblages::removeSphericalVoxelComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addAxisRotationComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                  const f64q& axisOrientation, f64 startAngle,
                                                                  f64 rotationalPeriod) {
    vecs::ComponentID arCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_AXISROTATION_NAME, entity);
    auto& arCmp = spaceSystem->m_axisRotationCT.get(arCmpId);
    arCmp.axisOrientation = axisOrientation;
    arCmp.currentRotation = startAngle;
    arCmp.period = rotationalPeriod;
    return arCmpId;
}

void SpaceSystemAssemblages::removeAxisRotationComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_AXISROTATION_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addSphericalTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                      vecs::ComponentID npComp,
                                                                      vecs::ComponentID arComp,
                                                                      PlanetGenData* planetGenData,
                                                                      vg::GLProgram* normalProgram,
                                                                      vg::TextureRecycler* normalMapRecycler) {
    vecs::ComponentID stCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, entity);
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

void SpaceSystemAssemblages::removeSphericalTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    auto& stcmp = spaceSystem->m_sphericalTerrainCT.getFromEntity(entity);

    delete stcmp.meshManager;
    delete stcmp.gpuGenerator;
    delete stcmp.cpuGenerator;
    delete stcmp.rpcDispatcher;
    delete stcmp.sphericalTerrainData;
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, entity);
}

/// Spherical terrain component
vecs::ComponentID SpaceSystemAssemblages::addFarTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                  SphericalTerrainComponent& parentCmp,
                                                                  WorldCubeFace face) {
    vecs::ComponentID ftCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_FARTERRAIN_NAME, entity);
    auto& ftCmp = spaceSystem->m_farTerrainCT.get(ftCmpId);

    ftCmp.planetGenData = parentCmp.planetGenData;
    ftCmp.meshManager = parentCmp.meshManager;
    ftCmp.gpuGenerator = parentCmp.gpuGenerator;
    ftCmp.cpuGenerator = parentCmp.cpuGenerator;
    ftCmp.rpcDispatcher = parentCmp.rpcDispatcher;
    ftCmp.sphericalTerrainData = parentCmp.sphericalTerrainData;

    ftCmp.face = face;
    ftCmp.alpha = TERRAIN_INC_START_ALPHA;

    return ftCmpId;
}

void SpaceSystemAssemblages::removeFarTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    auto& ftcmp = spaceSystem->m_farTerrainCT.getFromEntity(entity);

    if (ftcmp.patches) delete[] ftcmp.patches;
    ftcmp.patches = nullptr;
    ftcmp.gpuGenerator = nullptr;
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_FARTERRAIN_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addSphericalGravityComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                        vecs::ComponentID npComp, f64 radius, f64 mass) {
    vecs::ComponentID sgCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, entity);
    auto& sgCmp = spaceSystem->m_sphericalGravityCT.get(sgCmpId);
    sgCmp.namePositionComponent = npComp;
    sgCmp.radius = radius;
    sgCmp.mass = mass;
    return sgCmpId;
}

void SpaceSystemAssemblages::removeSphericalGravityComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addNamePositionComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                  const nString& name, const f64v3& position) {
    vecs::ComponentID npCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, entity);
    auto& npCmp = spaceSystem->m_namePositionCT.get(npCmpId);
    npCmp.name = name;
    npCmp.position = position;
    return npCmpId;
}

void SpaceSystemAssemblages::removeNamePositionComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addOrbitComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                           f64 eccentricity, f64 orbitalPeriod,
                                                           const ui8v4& pathColor, const f64q& orientation) {
    vecs::ComponentID oCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_ORBIT_NAME, entity);
    auto& oCmp = spaceSystem->m_orbitCT.get(oCmpId);
    oCmp.eccentricity = eccentricity;
    oCmp.orbitalPeriod = orbitalPeriod;
    oCmp.pathColor = pathColor;
    oCmp.orientation = orientation;
    return oCmpId;
}

void SpaceSystemAssemblages::removeOrbitComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_ORBIT_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addSpaceLightComponent(SpaceSystem* spaceSystem, vecs::EntityID entity, vecs::ComponentID npCmp, color3 color, f32 intensity) {
    vecs::ComponentID slCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_SPACELIGHT_NAME, entity);
    auto& slCmp = spaceSystem->m_spaceLightCT.get(slCmpId);
    slCmp.color = color;
    slCmp.intensity = intensity;
    slCmp.parentNpId = npCmp;
    return slCmpId;
}

void SpaceSystemAssemblages::removeSpaceLightComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_SPACELIGHT_NAME, entity);
}
