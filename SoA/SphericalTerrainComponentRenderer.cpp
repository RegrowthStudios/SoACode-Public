#include "stdafx.h"
#include "SphericalTerrainComponentRenderer.h"

#include "Camera.h"
#include "SpaceSystemComponents.h"
#include "SpaceSystemComponents.h"
#include "SphericalTerrainMeshManager.h"
#include "SphericalTerrainPatch.h"

#include <Vorb/io/IOManager.h>
#include <Vorb/graphics/GLProgram.h>
#include <glm/gtx/quaternion.hpp>

#define KM_PER_VOXEL 0.0005

SphericalTerrainComponentRenderer::~SphericalTerrainComponentRenderer() {
    if (m_farTerrainProgram) {
        delete m_farTerrainProgram;
    }
}

void SphericalTerrainComponentRenderer::draw(SphericalTerrainComponent& cmp, const Camera* camera,
                                             const Camera* voxelCamera,
                                             vg::GLProgram* terrainProgram,
                                             vg::GLProgram* waterProgram,
                                             const NamePositionComponent* npComponent,
                                             const AxisRotationComponent* arComponent) {
    if (cmp.patches) {
        f32m4 rotationMatrix = f32m4(glm::toMat4(arComponent->currentOrientation));

        f64v3 relativeCameraPos = camera->getPosition() - npComponent->position;

        // Draw spherical patches
        cmp.meshManager->drawSphericalMeshes(relativeCameraPos, camera,
                                             rotationMatrix, terrainProgram, waterProgram);
    }

    if (voxelCamera) {
        // Lazy shader init
        if (!m_farTerrainProgram) {
            buildFarTerrainShaders();
        }
        glDisable(GL_CULL_FACE);
        f64v3 relativeCameraPos = voxelCamera->getPosition() * KM_PER_VOXEL;
        // Draw far patches
        cmp.meshManager->drawFarMeshes(relativeCameraPos, voxelCamera,
                                       m_farTerrainProgram, waterProgram);
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

    vio::IOManager iom;
    nString vertSource;
    nString fragSource;
    iom.readFileToString("Shaders/SphericalTerrain/FarTerrain.vert", vertSource);
    iom.readFileToString("Shaders/SphericalTerrain/FarTerrain.frag", fragSource);
    m_farTerrainProgram = new vg::GLProgram(true);
    m_farTerrainProgram->addShader(vg::ShaderType::VERTEX_SHADER, vertSource.c_str());
    m_farTerrainProgram->addShader(vg::ShaderType::FRAGMENT_SHADER, fragSource.c_str());
    m_farTerrainProgram->setAttributes(sphericalAttribs);
    m_farTerrainProgram->link();
    m_farTerrainProgram->initUniforms();

    m_farTerrainProgram->use();
    glUniform1i(m_farTerrainProgram->getUniform("unNormalMap"), 0);
    glUniform1i(m_farTerrainProgram->getUniform("unColorMap"), 1);
    glUniform1i(m_farTerrainProgram->getUniform("unTexture"), 2);
    glUniform1f(m_farTerrainProgram->getUniform("unTexelWidth"), (float)PATCH_NORMALMAP_WIDTH);
    m_farTerrainProgram->unuse();
}
