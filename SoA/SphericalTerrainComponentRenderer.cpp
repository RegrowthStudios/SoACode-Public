#include "stdafx.h"
#include "SphericalTerrainComponentRenderer.h"

#include "Camera.h"
#include "SpaceSystemComponents.h"
#include "SpaceSystemComponents.h"
#include "TerrainPatchMeshManager.h"
#include "TerrainPatch.h"

#include <glm/gtx/quaternion.hpp>

#include <Vorb/io/IOManager.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/utils.h>

SphericalTerrainComponentRenderer::~SphericalTerrainComponentRenderer() {
    if (m_farTerrainProgram) {
        delete m_farTerrainProgram;
    }
}

void SphericalTerrainComponentRenderer::draw(SphericalTerrainComponent& cmp, const Camera* camera,
                                             const Camera* voxelCamera,
                                             vg::GLProgram* terrainProgram,
                                             vg::GLProgram* waterProgram,
                                             const f64v3& lightPos,
                                             const SpaceLightComponent* spComponent,
                                             const NamePositionComponent* npComponent,
                                             const AxisRotationComponent* arComponent) {

    f32v3 lightDir = f32v3(glm::normalize(lightPos - npComponent->position));
    f64v3 relativeCameraPos = camera->getPosition() - npComponent->position;
    if (cmp.patches) {
        
        // Draw spherical patches
        cmp.meshManager->drawSphericalMeshes(relativeCameraPos, camera,
                                             arComponent->currentOrientation,
                                             terrainProgram, waterProgram,
                                             lightDir);
    }

    if (voxelCamera) {
      //  const f32v3 up(0.0f, 1.0f, 0.0f);
      //  const f32v3 normalizedPos = f32v3(glm::normalize(arComponent->currentOrientation * relativeCameraPos));
        // Calculate relative light position
        // TODO(Ben): Worry if they are exactly the same
        lightDir = f32v3(glm::inverse(arComponent->currentOrientation) * f64v3(lightDir));
        // Lazy shader init
        if (!m_farTerrainProgram) {
            buildFarTerrainShaders();
        }
        glDisable(GL_CULL_FACE);
        f64v3 relativeCameraPos = voxelCamera->getPosition() * KM_PER_VOXEL;
        // Draw far patches
        cmp.meshManager->drawFarMeshes(relativeCameraPos, voxelCamera,
                                       m_farTerrainProgram, m_farWaterProgram,
                                       lightDir);
    }
}

void SphericalTerrainComponentRenderer::buildFarTerrainShaders() {
    // Attributes for spherical terrain
    std::vector<nString> sphericalAttribs;
    sphericalAttribs.push_back("vPosition");
    sphericalAttribs.push_back("vTangent");
    sphericalAttribs.push_back("vUV");
    sphericalAttribs.push_back("vColor");
    sphericalAttribs.push_back("vNormUV");
    sphericalAttribs.push_back("vTemp_Hum");

    // Attributes for spherical water
    std::vector<nString> sphericalWaterAttribs;
    sphericalWaterAttribs.push_back("vPosition");
    sphericalWaterAttribs.push_back("vTangent");
    sphericalWaterAttribs.push_back("vColor_Temp");
    sphericalWaterAttribs.push_back("vUV");
    sphericalWaterAttribs.push_back("vDepth");

    vio::IOManager iom;
    nString vertSource;
    nString fragSource;
    // Build terrain shader
    iom.readFileToString("Shaders/SphericalTerrain/FarTerrain.vert", vertSource);
    iom.readFileToString("Shaders/SphericalTerrain/SphericalTerrain.frag", fragSource);
    m_farTerrainProgram = new vg::GLProgram(true);
    m_farTerrainProgram->addShader(vg::ShaderType::VERTEX_SHADER, vertSource.c_str());
    m_farTerrainProgram->addShader(vg::ShaderType::FRAGMENT_SHADER, fragSource.c_str());
    m_farTerrainProgram->setAttributes(sphericalAttribs);
    m_farTerrainProgram->link();
    m_farTerrainProgram->initUniforms();
    // Set constant uniforms
    m_farTerrainProgram->use();
    glUniform1i(m_farTerrainProgram->getUniform("unNormalMap"), 0);
    glUniform1i(m_farTerrainProgram->getUniform("unColorMap"), 1);
    glUniform1i(m_farTerrainProgram->getUniform("unTexture"), 2);
    glUniform1f(m_farTerrainProgram->getUniform("unTexelWidth"), (float)PATCH_NORMALMAP_WIDTH);
    m_farTerrainProgram->unuse();

    // Build water shader
    iom.readFileToString("Shaders/SphericalTerrain/FarWater.vert", vertSource);
    iom.readFileToString("Shaders/SphericalTerrain/SphericalWater.frag", fragSource);
    m_farWaterProgram = new vg::GLProgram(true);
    m_farWaterProgram->addShader(vg::ShaderType::VERTEX_SHADER, vertSource.c_str());
    m_farWaterProgram->addShader(vg::ShaderType::FRAGMENT_SHADER, fragSource.c_str());
    m_farWaterProgram->setAttributes(sphericalWaterAttribs);
    m_farWaterProgram->link();
    m_farWaterProgram->initUniforms();
    // Set constant uniforms
    m_farWaterProgram->use();
    glUniform1i(m_farWaterProgram->getUniform("unNormalMap"), 0);
    glUniform1i(m_farWaterProgram->getUniform("unColorMap"), 1);
    m_farWaterProgram->unuse();
}
