#include "stdafx.h"
#include "FarTerrainComponentRenderer.h"

#include "Camera.h"
#include "SpaceSystemComponents.h"
#include "TerrainPatchMeshManager.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/utils.h>

FarTerrainComponentRenderer::~FarTerrainComponentRenderer() {
    delete m_farTerrainProgram;
    delete m_farWaterProgram;
}


void FarTerrainComponentRenderer::draw(FarTerrainComponent& cmp,
                                       const Camera* camera,
                                       const f64v3& lightPos,
                                       const SpaceLightComponent* spComponent,
                                       const AxisRotationComponent* arComponent) {
    if (camera) {
        // Calculate relative light position
        f32v3 lightDir = f32v3(glm::inverse(arComponent->currentOrientation) * f64v3(lightDir));
        // Lazy shader init
        if (!m_farTerrainProgram) {
            buildShaders();
        }
        glDisable(GL_CULL_FACE);
        f64v3 relativeCameraPos = camera->getPosition() * KM_PER_VOXEL;
        // Draw far patches
        cmp.meshManager->drawFarMeshes(relativeCameraPos, camera,
                                       m_farTerrainProgram, m_farWaterProgram,
                                       lightDir);
        glEnable(GL_CULL_FACE);
    }
}

void FarTerrainComponentRenderer::buildShaders() {
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
