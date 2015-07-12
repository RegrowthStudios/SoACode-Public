#include "stdafx.h"

#include "SpaceSystem.h"

#include <Vorb/TextureRecycler.hpp>
#include <Vorb/graphics/GLProgram.h>

SpaceSystem::SpaceSystem() : vecs::ECS() {
    // Add in component tables
    addComponentTable(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, &namePosition);
    addComponentTable(SPACE_SYSTEM_CT_AXISROTATION_NAME, &axisRotation);
    addComponentTable(SPACE_SYSTEM_CT_ORBIT_NAME, &orbit);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, &sphericalTerrain);
    addComponentTable(SPACE_SYSTEM_CT_GASGIANT_NAME, &gasGiant);
    addComponentTable(SPACE_SYSTEM_CT_STAR_NAME, &star);
    addComponentTable(SPACE_SYSTEM_CT_FARTERRAIN_NAME, &farTerrain);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, &sphericalGravity);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, &sphericalVoxel);
    addComponentTable(SPACE_SYSTEM_CT_SPACELIGHT_NAME, &spaceLight);
    addComponentTable(SPACE_SYSTEM_CT_ATMOSPHERE_NAME, &atmosphere);
    addComponentTable(SPACE_SYSTEM_CT_PLANETRINGS_NAME, &planetRings);
    addComponentTable(SPACE_SYSTEM_CT_CLOUDS_NAME, &clouds);
}

SpaceSystem::~SpaceSystem() {

    for (auto& it : sphericalVoxel) {
        sphericalVoxel.disposeComponent(sphericalVoxel.getComponentID(it.first), it.first);
    }
    for (auto& it : orbit) {
        orbit.disposeComponent(orbit.getComponentID(it.first), it.first);
    }
    for (auto& it : sphericalVoxel) {
        sphericalVoxel.disposeComponent(sphericalVoxel.getComponentID(it.first), it.first);
    }
    for (auto& it : sphericalTerrain) {
        sphericalTerrain.disposeComponent(sphericalTerrain.getComponentID(it.first), it.first);
    }
}
