#include "stdafx.h"
#include "FarTerrainComponentRenderer.h"

#include "Camera.h"
#include "PlanetData.h"
#include "ShaderLoader.h"
#include "SpaceSystemComponents.h"
#include "TerrainPatchMeshManager.h"
#include "VoxelSpaceUtils.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/ShaderManager.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/utils.h>

FarTerrainComponentRenderer::~FarTerrainComponentRenderer() {
    disposeShaders();
}

void FarTerrainComponentRenderer::draw(const FarTerrainComponent& cmp,
                                       const Camera* camera,
                                       const f64v3& lightDir,
                                       const SpaceLightComponent* spComponent,
                                       const AxisRotationComponent* arComponent,
                                       const AtmosphereComponent* aComponent) {
    // Get voxel position for quaternion calculation
    VoxelPosition3D pos;
    pos.pos = camera->getPosition();
    pos.face = cmp.face;

    // Lazy shader init
    if (!m_farTerrainProgram) {
        buildShaders();
    }
    f64v3 relativeCameraPos = camera->getPosition() * KM_PER_VOXEL;

    // Calculate relative light position
    f64v3 relLightDir = glm::inverse(arComponent->currentOrientation) * lightDir;
    relLightDir = glm::inverse(VoxelSpaceUtils::calculateVoxelToSpaceQuat(pos, cmp.sphericalTerrainData->radius * VOXELS_PER_KM)) * relLightDir;
    
    // Sort meshes
    cmp.meshManager->sortFarMeshes(relativeCameraPos);
    // Draw far patches
    if (cmp.alpha > 0.0f) {
        cmp.meshManager->drawFarMeshes(relativeCameraPos, camera,
                                       m_farTerrainProgram, m_farWaterProgram,
                                       f32v3(relLightDir),
                                       glm::min(cmp.alpha, 1.0f),
                                       cmp.planetGenData->radius,
                                       aComponent,
                                       (cmp.alpha >= 1.0f));
    }
}

void FarTerrainComponentRenderer::disposeShaders() {
    if (m_farTerrainProgram) {
        vg::ShaderManager::destroyProgram(&m_farTerrainProgram);
    }
    if (m_farWaterProgram) {
        vg::ShaderManager::destroyProgram(&m_farWaterProgram);
    }
}

void FarTerrainComponentRenderer::buildShaders() {

    m_farTerrainProgram = ShaderLoader::createProgramFromFile("Shaders/SphericalTerrain/FarTerrain.vert",
                                                              "Shaders/SphericalTerrain/SphericalTerrain.frag");
    // Set constant uniforms
    m_farTerrainProgram->use();
    glUniform1i(m_farTerrainProgram->getUniform("unNormalMap"), 0);
    glUniform1i(m_farTerrainProgram->getUniform("unColorMap"), 1);
    glUniform1i(m_farTerrainProgram->getUniform("unTexture"), 2);
    glUniform1f(m_farTerrainProgram->getUniform("unTexelWidth"), 1.0f / (float)PATCH_NORMALMAP_WIDTH);
    glUniform1f(m_farTerrainProgram->getUniform("unNormalmapWidth"), (float)(PATCH_NORMALMAP_WIDTH - 2) / (float)PATCH_NORMALMAP_WIDTH);
    m_farTerrainProgram->unuse();

    // Build water shader
   
    m_farWaterProgram = ShaderLoader::createProgramFromFile("Shaders/SphericalTerrain/FarWater.vert",
                                                            "Shaders/SphericalTerrain/SphericalWater.frag");
    // Set constant uniforms
    m_farWaterProgram->use();
    glUniform1i(m_farWaterProgram->getUniform("unNormalMap"), 0);
    glUniform1i(m_farWaterProgram->getUniform("unColorMap"), 1);
    m_farWaterProgram->unuse();
}
