#include "stdafx.h"

#include "SpaceSystem.h"
#include "SphericalTerrainGpuGenerator.h"

#include <Vorb/TextureRecycler.hpp>
#include <Vorb/graphics/GLProgram.h>

SpaceSystem::SpaceSystem() : vecs::ECS() {
    // Add in component tables
    addComponentTable(SPACE_SYSTEM_CT_NAMEPOSITIION_NAME, &m_namePositionCT);
    addComponentTable(SPACE_SYSTEM_CT_AXISROTATION_NAME, &m_axisRotationCT);
    addComponentTable(SPACE_SYSTEM_CT_ORBIT_NAME, &m_orbitCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALTERRAIN_NAME, &m_sphericalTerrainCT);
    addComponentTable(SPACE_SYSTEM_CT_FARTERRAIN_NAME, &m_farTerrainCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALGRAVITY_NAME, &m_sphericalGravityCT);
    addComponentTable(SPACE_SYSTEM_CT_SPHERICALVOXEL_NAME, &m_sphericalVoxelCT);
    addComponentTable(SPACE_SYSTEM_CT_SPACELIGHT_NAME, &m_spaceLightCT);
    addComponentTable(SPACE_SYSTEM_CT_ATMOSPHERE_NAME, &m_atmosphereCT);
   
    #define MAX_NORMAL_MAPS 512U
    normalMapRecycler = std::make_unique<vg::TextureRecycler>((ui32)PATCH_NORMALMAP_WIDTH,
                                                  (ui32)PATCH_NORMALMAP_WIDTH,
                                                  &vg::SamplerState::POINT_CLAMP,
                                                  0,
                                                  vg::TextureInternalFormat::RGB8,
                                                  MAX_NORMAL_MAPS);
}

SpaceSystem::~SpaceSystem() {
    if (normalMapGenProgram) {
        normalMapGenProgram->dispose();
    }
}
