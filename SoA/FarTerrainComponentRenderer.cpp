#include "stdafx.h"
#include "FarTerrainComponentRenderer.h"

#include "Camera.h"
#include "PlanetGenData.h"
#include "ShaderLoader.h"
#include "SpaceSystemComponents.h"
#include "TerrainPatchMeshManager.h"
#include "VoxelSpaceUtils.h"

#include <Vorb/os.h>
#include <Vorb/graphics/ShaderManager.h>
#include <Vorb/io/IOManager.h>
#include <Vorb/utils.h>

FarTerrainComponentRenderer::~FarTerrainComponentRenderer() {
    dispose();
}

void FarTerrainComponentRenderer::initGL() {
    if (!m_farTerrainProgram.isCreated()) {
        buildShaders();
    }
}

void FarTerrainComponentRenderer::draw(const FarTerrainComponent& cmp,
                                       const Camera* camera,
                                       const f64v3& lightDir,
                                       const f32 zCoef,
                                       const SpaceLightComponent* spComponent VORB_UNUSED,
                                       const AxisRotationComponent* arComponent,
                                       const AtmosphereComponent* aComponent) {
    // Get voxel position for quaternion calculation
    VoxelPosition3D pos;
    pos.pos = camera->getPosition();
    pos.face = cmp.face;

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
                                       (f32)cmp.planetGenData->radius,
                                       zCoef,
                                       aComponent,
                                       (cmp.alpha >= 1.0f));
    }
}

void FarTerrainComponentRenderer::dispose() {
    if (m_farTerrainProgram.isCreated()) m_farTerrainProgram.dispose();
    if (m_farWaterProgram.isCreated()) m_farWaterProgram.dispose();
}

void FarTerrainComponentRenderer::buildShaders() {

    m_farTerrainProgram = ShaderLoader::createProgramFromFile("Shaders/SphericalTerrain/FarTerrain.vert",
                                                              "Shaders/SphericalTerrain/SphericalTerrain.frag");
    // Set constant uniforms
    m_farTerrainProgram.use();
    glUniform1i(m_farTerrainProgram.getUniform("unColorMap"), 1);
    glUniform1i(m_farTerrainProgram.getUniform("unGrassTexture"), 2);
    glUniform1i(m_farTerrainProgram.getUniform("unRockTexture"), 3);
    m_farTerrainProgram.unuse();

    // Build water shader 
    m_farWaterProgram = ShaderLoader::createProgramFromFile("Shaders/SphericalTerrain/FarWater.vert",
                                                            "Shaders/SphericalTerrain/SphericalWater.frag");
    // Set constant uniforms
    m_farWaterProgram.use();
    glUniform1i(m_farWaterProgram.getUniform("unNormalMap"), 0);
    glUniform1i(m_farWaterProgram.getUniform("unColorMap"), 1);
    m_farWaterProgram.unuse();
}
