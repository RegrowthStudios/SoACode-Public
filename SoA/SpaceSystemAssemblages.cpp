#include "stdafx.h"
#include "SpaceSystemAssemblages.h"

#include "ChunkGrid.h"
#include "ChunkIOManager.h"
#include "ChunkAllocator.h"
#include "FarTerrainPatch.h"
#include "OrbitComponentUpdater.h"
#include "SoAState.h"
#include "SpaceSystem.h"
#include "SphericalTerrainComponentUpdater.h"
#include "SphericalHeightmapGenerator.h"

#include "TerrainPatchMeshManager.h"
#include "SpaceSystemAssemblages.h"
#include "SpaceSystemLoadStructs.h"

// TEMPORARY
#include "GameManager.h"
#include "PlanetGenData.h"

#define SEC_PER_HOUR 3600.0

Event<SphericalVoxelComponent&, vecs::EntityID> SpaceSystemAssemblages::onAddSphericalVoxelComponent;
Event<SphericalVoxelComponent&, vecs::EntityID> SpaceSystemAssemblages::onRemoveSphericalVoxelComponent;

vecs::EntityID SpaceSystemAssemblages::createOrbit(SpaceSystem* spaceSystem,
                                                   const SystemOrbitProperties* sysProps,
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
                                    const SystemOrbitProperties* sysProps,
                                    const PlanetProperties* properties,
                                    SystemBody* body,
                                    vcore::ThreadPool<WorkerData>* threadPool) {
    body->entity = spaceSystem->addEntity();
    const vecs::EntityID& id = body->entity;

    // const f64v3 up(0.0, 1.0, 0.0);
    vecs::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, properties->aTilt, properties->lNorth,
                                                       0.0, properties->rotationalPeriod * SEC_PER_HOUR);

    f64v3 tmpPos(0.0);
    vecs::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    addSphericalTerrainComponent(spaceSystem, id, npCmp, arCmp,
                                 properties->diameter * 0.5,
                                 properties->planetGenData,
                                 threadPool);

    f64 planetRadius = properties->diameter / 2.0;
    addSphericalGravityComponent(spaceSystem, id, npCmp, planetRadius, properties->mass);

    const AtmosphereProperties& at = properties->atmosphere;
    // Check if its active
    if (at.scaleDepth != -1.0f) {
        addAtmosphereComponent(spaceSystem, id, npCmp, (f32)planetRadius, (f32)(planetRadius * 1.025),
                               at.kr, at.km, at.g, at.scaleDepth,
                               at.waveLength);
    }

    const CloudsProperties& cl = properties->clouds;

    if (cl.density > 0.0f) {
        addCloudsComponent(spaceSystem, id, npCmp, (f32)planetRadius, (f32)(planetRadius * 0.0075), cl.color, cl.scale, cl.density);
    }

    SpaceSystemAssemblages::addOrbitComponent(spaceSystem, id, npCmp, sysProps->type, sysProps->e,
                                              sysProps->t, sysProps->n, sysProps->p,
                                              sysProps->i, sysProps->a);

    return id;
}

void SpaceSystemAssemblages::destroyPlanet(SpaceSystem* gameSystem VORB_UNUSED, vecs::EntityID planetEntity VORB_UNUSED) {
    // TODO: implement and remove VORB_UNUSED tags.
}

vecs::EntityID SpaceSystemAssemblages::createStar(SpaceSystem* spaceSystem,
                                  const SystemOrbitProperties* sysProps,
                                  const StarProperties* properties,
                                  SystemBody* body) {
    body->entity = spaceSystem->addEntity();
    const vecs::EntityID& id = body->entity;

    // const f64v3 up(0.0, 1.0, 0.0);
    vecs::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, properties->aTilt, properties->lNorth,
                                                       0.0, properties->rotationalPeriod * SEC_PER_HOUR);

    f64v3 tmpPos(0.0);
    vecs::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    f64 radius = properties->diameter / 2.0;
    addStarComponent(spaceSystem, id, npCmp, arCmp, properties->mass, radius, properties->surfaceTemperature);

    addSpaceLightComponent(spaceSystem, id, npCmp, color3(255, 255, 255), 1.0f);

    addSphericalGravityComponent(spaceSystem, id, npCmp, radius, properties->mass);

    SpaceSystemAssemblages::addOrbitComponent(spaceSystem, id, npCmp, sysProps->type, sysProps->e,
                                                     sysProps->t, sysProps->n, sysProps->p,
                                                     sysProps->i, sysProps->a);

    return id;
}

void SpaceSystemAssemblages::destroyStar(SpaceSystem* gameSystem VORB_UNUSED, vecs::EntityID planetEntity VORB_UNUSED) {
    // TODO: implement and remove VORB_UNUSED tags.
}

/// GasGiant entity
vecs::EntityID SpaceSystemAssemblages::createGasGiant(SpaceSystem* spaceSystem,
                                      const SystemOrbitProperties* sysProps,
                                      const GasGiantProperties* properties,
                                      SystemBody* body) {
    body->entity = spaceSystem->addEntity();
    const vecs::EntityID& id = body->entity;

    // const f64v3 up(0.0, 1.0, 0.0);
    vecs::ComponentID arCmp = addAxisRotationComponent(spaceSystem, id, properties->aTilt, properties->lNorth,
                                                       0.0, properties->rotationalPeriod * SEC_PER_HOUR);

    f64v3 tmpPos(0.0);
    vecs::ComponentID npCmp = addNamePositionComponent(spaceSystem, id, body->name, tmpPos);

    f64 radius = properties->diameter / 2.0;
    addGasGiantComponent(spaceSystem, id, npCmp, arCmp, properties->oblateness, radius,
                         properties->colorMap, properties->rings);

    addSphericalGravityComponent(spaceSystem, id, npCmp, radius, properties->mass);

    const AtmosphereProperties& at = properties->atmosphere;
    // Check if its active
    if (at.scaleDepth != -1.0f) {
        addAtmosphereComponent(spaceSystem, id, npCmp, (f32)radius, (f32)(radius * 1.025),
                               at.kr, at.km, at.g, at.scaleDepth,
                               at.waveLength, properties->oblateness);
    }

    if (properties->rings.size()) {
        addPlanetRingsComponent(spaceSystem, id, npCmp, properties->rings);
    }

    SpaceSystemAssemblages::addOrbitComponent(spaceSystem, id, npCmp, sysProps->type, sysProps->e,
                                                     sysProps->t, sysProps->n, sysProps->p,
                                                     sysProps->i, sysProps->a);

    return id;
}

void SpaceSystemAssemblages::destroyGasGiant(SpaceSystem* gameSystem VORB_UNUSED, vecs::EntityID planetEntity VORB_UNUSED) {
    // TODO: implement and remove VORB_UNUSED tags.
}

vecs::ComponentID SpaceSystemAssemblages::addAtmosphereComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                 vecs::ComponentID namePositionComponent, f32 planetRadius,
                                                                 f32 radius, f32 kr, f32 km, f32 g, f32 scaleDepth,
                                                                 f32v3 wavelength, f32 oblateness /*= 0.0f*/) {
    vecs::ComponentID aCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_ATMOSPHERE_NAME, entity);
    auto& aCmp = spaceSystem->atmosphere.get(aCmpId);
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
                                          vecs::ComponentID namePositionComponent, const Array<PlanetRingProperties>& rings) {
    vecs::ComponentID prCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_PLANETRINGS_NAME, entity);
    if (prCmpId == 0) {
        return 0;
    }
    auto& prCmp = spaceSystem->planetRings.get(prCmpId);
    prCmp.namePositionComponent = namePositionComponent;
    prCmp.rings.resize(rings.size());
    for (size_t i = 0; i < rings.size(); i++) {
        auto& r1 = prCmp.rings[i];
        auto& r2 = rings[i];
        r1.innerRadius = r2.innerRadius;
        r1.outerRadius = r2.outerRadius;
        r1.texturePath = r2.colorLookup;
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
    auto& cCmp = spaceSystem->clouds.get(cCmpId);
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

// TODO: Is this implementation complete?
vecs::ComponentID SpaceSystemAssemblages::addSphericalVoxelComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                      vecs::ComponentID sphericalTerrainComponent,
                                                                      vecs::ComponentID farTerrainComponent,
                                                                      vecs::ComponentID axisRotationComponent,
                                                                      vecs::ComponentID namePositionComponent,
                                                                      WorldCubeFace worldFace VORB_UNUSED,
                                                                      SoaState* soaState) {

    vecs::ComponentID svCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, entity);
    if (svCmpId == 0) {
        return 0;
    }
    auto& svcmp = spaceSystem->sphericalVoxel.get(svCmpId);

    auto& ftcmp = spaceSystem->farTerrain.get(farTerrainComponent);

    // Get component handles
    svcmp.sphericalTerrainComponent = sphericalTerrainComponent;
    svcmp.axisRotationComponent = axisRotationComponent;
    svcmp.namePositionComponent = namePositionComponent;
    svcmp.farTerrainComponent = farTerrainComponent;

    svcmp.voxelRadius = ftcmp.sphericalTerrainData->radius * VOXELS_PER_KM;

    svcmp.generator = ftcmp.cpuGenerator;
    svcmp.chunkIo = new ChunkIOManager("TESTSAVEDIR"); // TODO(Ben): Fix
    svcmp.blockPack = &soaState->blocks;

    svcmp.threadPool = soaState->threadPool;

    svcmp.chunkIo->beginThread();

    svcmp.chunkGrids = new ChunkGrid[6];
    for (int i = 0; i < 6; i++) {
        svcmp.chunkGrids[i].init(static_cast<WorldCubeFace>(i), svcmp.threadPool, 1, ftcmp.planetGenData, &soaState->chunkAllocator);
        svcmp.chunkGrids[i].blockPack = &soaState->blocks;
    }

    svcmp.planetGenData = ftcmp.planetGenData;
    svcmp.sphericalTerrainData = ftcmp.sphericalTerrainData;
    svcmp.saveFileIom = &soaState->saveFileIom;

    onAddSphericalVoxelComponent(svcmp, entity);
    return svCmpId;
}

void SpaceSystemAssemblages::removeSphericalVoxelComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    onRemoveSphericalVoxelComponent(spaceSystem->sphericalVoxel.getFromEntity(entity), entity);
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addAxisRotationComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                  f32 aTilt, f32 lNorth, f64 startAngle,
                                                                  f64 rotationalPeriod) {
    vecs::ComponentID arCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_AXISROTATION_NAME, entity);
    auto& arCmp = spaceSystem->axisRotation.get(arCmpId);
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
                                                                      vcore::ThreadPool<WorkerData>* threadPool) {
    vecs::ComponentID stCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, entity);
    auto& stCmp = spaceSystem->sphericalTerrain.get(stCmpId);
    
    stCmp.namePositionComponent = npComp;
    stCmp.axisRotationComponent = arComp;
    stCmp.planetGenData = planetGenData;

    if (planetGenData) {
        stCmp.meshManager = new TerrainPatchMeshManager(planetGenData);
        stCmp.cpuGenerator = new SphericalHeightmapGenerator;
        stCmp.cpuGenerator->init(planetGenData);
    }
    
    stCmp.radius = radius;
    stCmp.alpha = 1.0f;

    f64 patchWidth = (radius * 2.0) / ST_PATCH_ROW;
    stCmp.sphericalTerrainData = new TerrainPatchData(radius, patchWidth, stCmp.cpuGenerator,
                                                      stCmp.meshManager, threadPool);

    return stCmpId;
}

void SpaceSystemAssemblages::removeSphericalTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    // auto& stcmp = spaceSystem->sphericalTerrain.getFromEntity(entity);

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
    auto& sCmp = spaceSystem->star.get(sCmpId);

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
                                                               const nString& colorMapPath,
                                                               const Array<PlanetRingProperties>& rings) {
    vecs::ComponentID ggCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_GASGIANT_NAME, entity);
    auto& ggCmp = spaceSystem->gasGiant.get(ggCmpId);

    ggCmp.namePositionComponent = npComp;
    ggCmp.axisRotationComponent = arComp;
    ggCmp.oblateness = oblateness;
    ggCmp.radius = radius;
    ggCmp.colorMapPath = colorMapPath;
    ggCmp.rings = rings;
    return ggCmpId;
}

void SpaceSystemAssemblages::removeGasGiantComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_GASGIANT_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addFarTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                  SphericalTerrainComponent& parentCmp,
                                                                  WorldCubeFace face) {
    vecs::ComponentID ftCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_FARTERRAIN_NAME, entity);
    auto& ftCmp = spaceSystem->farTerrain.get(ftCmpId);

    ftCmp.planetGenData = parentCmp.planetGenData;
    ftCmp.meshManager = parentCmp.meshManager;
    ftCmp.cpuGenerator = parentCmp.cpuGenerator;
    ftCmp.sphericalTerrainData = parentCmp.sphericalTerrainData;

    ftCmp.face = face;
    ftCmp.alpha = TERRAIN_INC_START_ALPHA;

    return ftCmpId;
}

void SpaceSystemAssemblages::removeFarTerrainComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    auto& ftcmp = spaceSystem->farTerrain.getFromEntity(entity);

    if (ftcmp.patches) delete[] ftcmp.patches;
    ftcmp.patches = nullptr;
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_FARTERRAIN_NAME, entity);
}

vecs::ComponentID SpaceSystemAssemblages::addSphericalGravityComponent(SpaceSystem* spaceSystem, vecs::EntityID entity,
                                                                        vecs::ComponentID npComp, f64 radius, f64 mass) {
    vecs::ComponentID sgCmpId = spaceSystem->addComponent(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, entity);
    auto& sgCmp = spaceSystem->sphericalGravity.get(sgCmpId);
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
    auto& npCmp = spaceSystem->namePosition.get(npCmpId);
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
    auto& oCmp = spaceSystem->orbit.get(oCmpId);
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
    auto& slCmp = spaceSystem->spaceLight.get(slCmpId);
    slCmp.color = color;
    slCmp.intensity = intensity;
    slCmp.npID = npCmp;
    return slCmpId;
}

void SpaceSystemAssemblages::removeSpaceLightComponent(SpaceSystem* spaceSystem, vecs::EntityID entity) {
    spaceSystem->deleteComponent(SPACE_SYSTEM_CT_SPACELIGHT_NAME, entity);
}
