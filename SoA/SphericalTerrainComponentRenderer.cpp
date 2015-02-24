#include "stdafx.h"
#include "SphericalTerrainComponentRenderer.h"

#include "Camera.h"
#include "SpaceSystemComponents.h"
#include "TerrainPatchMeshManager.h"
#include "TerrainPatch.h"

#include <glm/gtx/quaternion.hpp>

#include <Vorb/io/IOManager.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/utils.h>

SphericalTerrainComponentRenderer::~SphericalTerrainComponentRenderer() {
    if (m_terrainProgram) {
        m_terrainProgram->dispose();
        m_waterProgram->dispose();
        delete m_terrainProgram;
        delete m_waterProgram;
    }
}

void SphericalTerrainComponentRenderer::draw(SphericalTerrainComponent& cmp,
                                             const Camera* camera,
                                             const f32v3& lightDir,
                                             const f64v3& position,
                                             const SpaceLightComponent* spComponent,
                                             const AxisRotationComponent* arComponent) {

    if (cmp.patches) {
        // Lazy shader init
        if (!m_terrainProgram) {
            buildShaders();
        }
        
        f64v3 relativeCameraPos = camera->getPosition() - position;
        // Draw spherical patches
        cmp.meshManager->drawSphericalMeshes(relativeCameraPos, camera,
                                             arComponent->currentOrientation,
                                             m_terrainProgram, m_waterProgram,
                                             lightDir,
                                             cmp.alpha);
    }
}

void SphericalTerrainComponentRenderer::buildShaders() {
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
    iom.readFileToString("Shaders/SphericalTerrain/SphericalTerrain.vert", vertSource);
    iom.readFileToString("Shaders/SphericalTerrain/SphericalTerrain.frag", fragSource);
    m_terrainProgram = new vg::GLProgram(true);
    m_terrainProgram->addShader(vg::ShaderType::VERTEX_SHADER, vertSource.c_str());
    m_terrainProgram->addShader(vg::ShaderType::FRAGMENT_SHADER, fragSource.c_str());
    m_terrainProgram->setAttributes(sphericalAttribs);
    m_terrainProgram->link();
    m_terrainProgram->initAttributes();
    m_terrainProgram->initUniforms();
    // Set constant uniforms
    m_terrainProgram->use();
    glUniform1i(m_terrainProgram->getUniform("unNormalMap"), 0);
    glUniform1i(m_terrainProgram->getUniform("unColorMap"), 1);
    glUniform1i(m_terrainProgram->getUniform("unTexture"), 2);
    glUniform1f(m_terrainProgram->getUniform("unTexelWidth"), (float)PATCH_NORMALMAP_WIDTH);
    m_terrainProgram->unuse();

    // Build water shader
    iom.readFileToString("Shaders/SphericalTerrain/SphericalWater.vert", vertSource);
    iom.readFileToString("Shaders/SphericalTerrain/SphericalWater.frag", fragSource);
    m_waterProgram = new vg::GLProgram(true);
    m_waterProgram->addShader(vg::ShaderType::VERTEX_SHADER, vertSource.c_str());
    m_waterProgram->addShader(vg::ShaderType::FRAGMENT_SHADER, fragSource.c_str());
    m_waterProgram->setAttributes(sphericalWaterAttribs);
    m_waterProgram->link();
    m_waterProgram->initAttributes();
    m_waterProgram->initUniforms();
    // Set constant uniforms
    m_waterProgram->use();
    glUniform1i(m_waterProgram->getUniform("unNormalMap"), 0);
    glUniform1i(m_waterProgram->getUniform("unColorMap"), 1);
    m_waterProgram->unuse();
}
