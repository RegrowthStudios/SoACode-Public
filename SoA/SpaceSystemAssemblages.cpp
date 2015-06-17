#include "stdafx.h"
#include "SpaceSystemAssemblages.h"

#include "NChunkGrid.h"
#include "ChunkIOManager.h"
#include "ChunkListManager.h"
#include "ChunkAllocator.h"
#include "FarTerrainPatch.h"
#include "OrbitComponentUpdater.h"
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
#include "PlanetData.h"

#include <glm/gtx/quaternion.hpp>

#define SEC_PER_HOUR 3600.0

vecs::EntityID SpaceSystemAssemblages::createOrbit(SpaceSystem* spaceSystem,
                           const SystemBodyKegProperties* sysProps,
                           SystemBody* body, f64 bodyRadius) {
    body->entity = spaceSystem->addEntity();
    const vecs::EntityID& id = body->entity;

    f64v3 tmpPos(0.0);
    vecs::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    SpaceSystemAssemblages::addOrbitComponent(spaceSystem, id, npCmp, sysProps->type, sysProps->e,
                                              sysProps->t, sysProps->n, sysProps->p,
                                              sysProps->i, sysProps->a);

    addSphericalGravityComponent(spaceSystem, id, npCmp, bodyRadius, body->mass);

    return id;
}

vecs::EntityID SpaceSystemAssemblages::createPlanet(SpaceSystem* spaceSystem,
                                    const SystemBodyKegProperties* sysProps,
                                    const PlanetKegProperties* properties,
                                    SystemBody* body) {
    body->entity = spaceSystem->addEntity();
    const vecs::EntityID& id = body->entity;

    const f64v3 up(0.0, 1.0, 0.0);
    vecs::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, properties->aTilt, properties->lNorth,
                                                       0.0, properties->rotationalPeriod * SEC_PER_HOUR);

    f64v3 tmpPos(0.0);
    vecs::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    addSphericalTerrainComponent(spaceSystem, id, npCmp, arCmp,
                                 properties->diameter * 0.5,
                                 properties->planetGenData,
                                 &spaceSystem->normalMapGenProgram,
                                 spaceSystem->normalMapRecycler.get());

    f64 planetRadius = properties->diameter / 2.0;
    addSphericalGravityComponent(spaceSystem, id, npCmp, planetRadius, properties->mass);

    const AtmosphereKegProperties& at = properties->atmosphere;
    // Check if its active
    if (at.scaleDepth != -1.0f) {
        addAtmosphereComponent(spaceSystem, id, npCmp, (f32)planetRadius, (f32)(planetRadius * 1.025),
                               at.kr, at.km, at.g, at.scaleDepth,
                               at.waveLength);
    }

    const CloudsKegProperties& cl = properties->clouds;

    if (cl.density > 0.0f) {
        addCloudsComponent(spaceSystem, id, npCmp, (f32)planetRadius, (f32)(planetRadius * 0.0075), cl.color, cl.scale, cl.density);
    }

    SpaceSystemAssemblages::addOrbitComponent(spaceSystem, id, npCmp, sysProps->type, sysProps->e,
                                              sysProps->t, sysProps->n, sysProps->p,
                                              sysProps->i, sysProps->a);

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
    vecs::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, properties->aTilt, properties->lNorth,
                                                       0.0, properties->rotationalPeriod * SEC_PER_HOUR);

    f64v3 tmpPos(0.0);
    vecs::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    f64 radius = properties->diameter / 2.0;
    addStarComponent(spaceSystem, id, npCmp, arCmp, properties->mass, radius, properties->surfaceTemperature);

    addSpaceLightComponent(spaceSystem, id, npCmp, color3(255, 255, 255), 1.0f);

    addSphericalGravityComponent(spaceSystem, id, npCmp, radius, properties->mass);

    return SpaceSystemAssemblages::addOrbitComponent(spaceSystem, id, npCmp, sysProps->type, sysProps->e,
                                                     sysProps->t, sysProps->n, sysProps->p,
                                                     sysProps->i, sysProps->a);

    return id;
}

void SpaceSystemAssemblages::destroyStar(SpaceSystem* gameSystem, vecs::EntityID planetEntity) {
    // TODO: implement
}

/// GasGiant entity
vecs::EntityID SpaceSystemAssemblages::createGasGiant(SpaceSystem* spaceSystem,
                                      const SystemBodyKegProperties* sysProps,
                                      const GasGiantKegProperties* properties,
                                      SystemBody* body,
                                      VGTexture colorMap) {
    body->entity = spaceSystem->addEntity();
    const vecs::EntityID& id = body->entity;

    const f64v3 up(0.0, 1.0, 0.0);
    vecs::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, properties->aTilt, properties->lNorth,
                                                       0.0, properties->rotationalPeriod * SEC_PER_HOUR);

    f64v3 tmpPos(0.0);
    vecs::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    f64 radius = properties->diameter / 2.0;
    addGasGiantComponent(spaceSystem, id, npCmp, arCmp, properties->oblateness, radius, colorMap);

    addSphericalGravityComponent(spaceSystem, id, npCmp, radius, properties->mass);

    const AtmosphereKegProperties& at = properties->atmosphere;
    // Check if its active
    if (at.scaleDepth != -1.0f) {
        addAtmosphereComponent(spaceSystem, id, npCmp, (f32)radius, (f32)(radius * 1.025),
                               at.kr, at.km, at.g, at.scaleDepth,
                               at.waveLength, properties->oblateness);
    }

    if (properties->rings.size()) {
        addPlanetRingsComponent(spaceSystem, id, npCmp, properties->rings);
    }

    return SpaceSystemAssemblages::addOrbitComponent(spaceSystem, id, npCmp, sysProps->type, sysProps->e,
                                                     sysProps->t, sysProps->n, sysProps->p,
                                                     sysProps->i, sysProps->a);

    return id;
}

void SpaceSystemAssemblages::destroyGasGiant(SpaceSystem* gameSystem, vecs::EntityID planetEntity) {
    // TODO: implement
}

vecs::ComponentID SpaceSystemAssemblages::addAtmosphereComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                 vecs::ComponentID namePositionComponent, f32 planetRadius,
                                                                 f32 radius, f32 kr, f32 km, f32 g, f32 scaleDepth,
                                                                 f32v3 wavelength, f32 oblateness /*= 0.0f*/) {
    vecs::ComponentID aCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_ATMOSPHERE_NAME, entity);
    auto& aCmp = spaceSystem->m_atmosphereCT.get(aCmpId);
    aCmp.namePositionComponent = namePositionComponent;
    aCmp.planetRadius = planetRadius;
    aCmp.radius = radius;
    aCmp.oblateness = oblateness;
    aCmp.kr = kr;
    aCmp.km = km;
    aCmp.g = g;
    aCmp.scaleDepth = scaleDepth;
    aCmp.invWavelength4 = f32v3(1.0f / powf(wavelength.r, 4.0f),
                                1.0f / powf(wavelength.g, 4.0f),
                                1.0f / powf(wavelength.b, 4.0f));
    return aCmpId;
}

void SpaceSystemAssemblages::removeAtmosphereComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_ATMOSPHERE_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addPlanetRingsComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                          vecs::ComponentID namePositionComponent, const Array<PlanetRingKegProperties>& rings) {
    vecs::ComponentID prCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_PLANETRINGS_NAME, entity);
    if (prCmpId == 0) {
        return 0;
    }
    auto& prCmp = spaceSystem->m_planetRingCT.get(prCmpId);
    prCmp.namePositionComponent = namePositionComponent;
    prCmp.rings.resize(rings.size());
    for (size_t i = 0; i < rings.size(); i++) {
        auto& r1 = prCmp.rings[i];
        auto& r2 = rings[i];
        r1.innerRadius = r2.innerRadius;
        r1.outerRadius = r2.outerRadius;
        r1.colorLookup = r2.texture;
        r1.orientation = glm::angleAxis((f64)r2.lNorth, f64v3(0.0, 1.0, 0.0)) * glm::angleAxis((f64)r2.aTilt, f64v3(1.0, 0.0, 0.0));
    }
    return prCmpId;
}

void  SpaceSystemAssemblages::removePlanetRingsComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_PLANETRINGS_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addCloudsComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                            vecs::ComponentID namePositionComponent, f32 planetRadius,
                                                            f32 height, f32v3 color, f32v3 scale, float density) {

    vecs::ComponentID cCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_CLOUDS_NAME, entity);
    auto& cCmp = spaceSystem->m_cloudsCT.get(cCmpId);
    cCmp.namePositionComponent = namePositionComponent;
    cCmp.planetRadius = planetRadius;
    cCmp.height = height;
    cCmp.color = color;
    cCmp.scale = scale;
    cCmp.density = density;
    return cCmpId;
}

void SpaceSystemAssemblages::removeCloudsComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_CLOUDS_NAME, entity);
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

    svcmp.generator = ftcmp.gpuGenerator;
    svcmp.chunkIo = new ChunkIOManager("TESTSAVEDIR"); // TODO(Ben): Fix
    svcmp.chunkListManager = new ChunkListManager();
    svcmp.chunkAllocator = new PagedChunkAllocator();
    svcmp.chunkMeshManager = soaState->chunkMeshManager;
    svcmp.blockPack = &soaState->blocks;

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

    svcmp.chunkGrids = new NChunkGrid[6];
    for (int i = 0; i < 6; i++) {
        svcmp.chunkGrids[i].init(static_cast<WorldCubeFace>(i), svcmp.chunkAllocator, svcmp.threadPool, 1, ftcmp.planetGenData);
    }

    svcmp.planetGenData = ftcmp.planetGenData;
    svcmp.sphericalTerrainData = ftcmp.sphericalTerrainData;
    svcmp.saveFileIom = &soaState->saveFileIom;

    return svCmpId;
}

void SpaceSystemAssemblages::removeSphericalVoxelComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addAxisRotationComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                  f32 aTilt, f32 lNorth, f64 startAngle,
                                                                  f64 rotationalPeriod) {
    vecs::ComponentID arCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_AXISROTATION_NAME, entity);
    auto& arCmp = spaceSystem->m_axisRotationCT.get(arCmpId);
    arCmp.tilt = aTilt;
    arCmp.axisOrientation = glm::angleAxis((f64)lNorth, f64v3(0.0, 1.0, 0.0)) * glm::angleAxis((f64)aTilt, f64v3(1.0, 0.0, 0.0));
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
                                                                      f64 radius,
                                                                      PlanetGenData* planetGenData,
                                                                      vg::GLProgram* normalProgram,
                                                                      vg::TextureRecycler* normalMapRecycler) {
    vecs::ComponentID stCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, entity);
    auto& stCmp = spaceSystem->m_sphericalTerrainCT.get(stCmpId);
    
    stCmp.namePositionComponent = npComp;
    stCmp.axisRotationComponent = arComp;
    stCmp.planetGenData = planetGenData;

    if (planetGenData) {
        stCmp.meshManager = new TerrainPatchMeshManager(planetGenData,
                                                        normalMapRecycler);
        stCmp.gpuGenerator = new SphericalTerrainGpuGenerator(stCmp.meshManager,
                                                              planetGenData,
                                                              normalProgram, normalMapRecycler);
        stCmp.cpuGenerator = new SphericalTerrainCpuGenerator;
        stCmp.cpuGenerator->init(planetGenData);
        stCmp.rpcDispatcher = new TerrainRpcDispatcher(stCmp.gpuGenerator, stCmp.cpuGenerator);
    }
    
    stCmp.radius = radius;
    stCmp.alpha = 1.0f;

    f64 patchWidth = (radius * 2.0) / ST_PATCH_ROW;
    stCmp.sphericalTerrainData = new TerrainPatchData(radius, patchWidth);

    return stCmpId;
}

void SpaceSystemAssemblages::removeSphericalTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    auto& stcmp = spaceSystem->m_sphericalTerrainCT.getFromEntity(entity);

    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, entity);
}

/// Star Component
vecs::ComponentID SpaceSystemAssemblages::addStarComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                          vecs::ComponentID npComp,
                                          vecs::ComponentID arComp,
                                          f64 mass,
                                          f64 radius,
                                          f64 temperature) {
    vecs::ComponentID sCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_STAR_NAME, entity);
    auto& sCmp = spaceSystem->m_starCT.get(sCmpId);

    sCmp.namePositionComponent = npComp;
    sCmp.axisRotationComponent = arComp;
    sCmp.radius = radius;
    sCmp.temperature = temperature;
    sCmp.mass = mass;
    sCmp.occlusionQuery[0] = 0;
    sCmp.occlusionQuery[1] = 0;

    return sCmpId;
}
void SpaceSystemAssemblages::removeStarComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_STAR_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addGasGiantComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                               vecs::ComponentID npComp,
                                                               vecs::ComponentID arComp,
                                                               f32 oblateness,
                                                               f64 radius,
                                                               VGTexture colorMap) {
    vecs::ComponentID ggCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_GASGIANT_NAME, entity);
    auto& ggCmp = spaceSystem->m_gasGiantCT.get(ggCmpId);

    ggCmp.namePositionComponent = npComp;
    ggCmp.axisRotationComponent = arComp;
    ggCmp.oblateness = oblateness;
    ggCmp.radius = radius;
    ggCmp.colorMap = colorMap;
    return ggCmpId;
}

void SpaceSystemAssemblages::removeGasGiantComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_GASGIANT_NAME, entity);
}

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
                                                            vecs::ComponentID npComp, SpaceObjectType oType,
                                                            f64 eccentricity, f64 orbitalPeriod,
                                                            f64 ascendingLong, f64 periapsisLong,
                                                            f64 inclination, f64 trueAnomaly) {
    vecs::ComponentID oCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_ORBIT_NAME, entity);
    auto& oCmp = spaceSystem->m_orbitCT.get(oCmpId);
    oCmp.e = eccentricity;
    oCmp.t = orbitalPeriod;
    oCmp.npID = npComp;
    oCmp.o = ascendingLong * DEG_TO_RAD;
    oCmp.p = periapsisLong * DEG_TO_RAD;
    oCmp.i = inclination * DEG_TO_RAD;
    oCmp.startMeanAnomaly = trueAnomaly * DEG_TO_RAD;
    oCmp.type = oType;

    // Get the path color
    std::pair<f32v4, f32v4> pathColor(f32v4(0.0f), f32v4(0.0f));
    switch (oType) {
        case SpaceObjectType::STAR:
            pathColor = spaceSystem->pathColorMap["Star"]; break;
        case SpaceObjectType::PLANET:
            pathColor = spaceSystem->pathColorMap["Planet"]; break;
        case SpaceObjectType::DWARF_PLANET:
            pathColor = spaceSystem->pathColorMap["DwarfPlanet"]; break;
        case SpaceObjectType::MOON:
            pathColor = spaceSystem->pathColorMap["Moon"]; break;
        case SpaceObjectType::DWARF_MOON:
            pathColor = spaceSystem->pathColorMap["DwarfMoon"]; break;
        case SpaceObjectType::ASTEROID:
            pathColor = spaceSystem->pathColorMap["Asteroid"]; break;
        case SpaceObjectType::COMET:
            pathColor = spaceSystem->pathColorMap["Comet"]; break;
        default:
            break;
    }
    oCmp.pathColor[0] = pathColor.first;
    oCmp.pathColor[1] = pathColor.second;

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
    slCmp.npID = npCmp;
    return slCmpId;
}

void SpaceSystemAssemblages::removeSpaceLightComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_SPACELIGHT_NAME, entity);
}
