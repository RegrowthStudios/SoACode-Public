#include "stdafx.h"

#include "SpaceSystem.h"

#include <Vorb/TextureRecycler.hpp>
#include <Vorb/graphics/GLProgram.h>

SpaceSystem::SpaceSystem() : vecs::ECS() {
    // Add in component tables
    addComponentTable(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, &m_namePositionCT);
    addComponentTable(SPACE_SYSTEM_CT_AXISROTATION_NAME, &m_axisRotationCT);
    addComponentTable(SPACE_SYSTEM_CT_ORBIT_NAME, &m_orbitCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, &m_sphericalTerrainCT);
    addComponentTable(SPACE_SYSTEM_CT_GASGIANT_NAME, &m_gasGiantCT);
    addComponentTable(SPACE_SYSTEM_CT_STAR_NAME, &m_starCT);
    addComponentTable(SPACE_SYSTEM_CT_FARTERRAIN_NAME, &m_farTerrainCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, &m_sphericalGravityCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, &m_sphericalVoxelCT);
    addComponentTable(SPACE_SYSTEM_CT_SPACELIGHT_NAME, &m_spaceLightCT);
    addComponentTable(SPACE_SYSTEM_CT_ATMOSPHERE_NAME, &m_atmosphereCT);
    addComponentTable(SPACE_SYSTEM_CT_PLANETRINGS_NAME, &m_planetRingCT);
    addComponentTable(SPACE_SYSTEM_CT_CLOUDS_NAME, &m_cloudsCT);
}

SpaceSystem::~SpaceSystem() {
    if (normalMapGenProgram.isCreated()) {
        normalMapGenProgram.dispose();
    }
    for (auto& it : m_sphericalVoxelCT) {
        m_sphericalVoxelCT.disposeComponent(m_sphericalVoxelCT.getComponentID(it.first), it.first);
    }
    for (auto& it : m_orbitCT) {
        m_orbitCT.disposeComponent(m_orbitCT.getComponentID(it.first), it.first);
    }
    for (auto& it : m_sphericalTerrainCT) {
        m_sphericalTerrainCT.disposeComponent(m_sphericalTerrainCT.getComponentID(it.first), it.first);
    }
}
