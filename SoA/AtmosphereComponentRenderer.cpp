#include "stdafx.h"
#include "AtmosphereComponentRenderer.h"

#include "SpaceSystem.h"
#include "RenderUtils.h"

#include "Camera.h"
#include <Vorb/io/IOManager.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/MeshGenerators.h>
#include <Vorb/graphics/GpuMemory.h>

#define ICOSPHERE_SUBDIVISIONS 6

AtmosphereComponentRenderer::AtmosphereComponentRenderer() {
    // Empty
}

AtmosphereComponentRenderer::~AtmosphereComponentRenderer() {
    if (m_program) {
        m_program->dispose();
        delete m_program;
    }
    if (m_icoVbo) {
        vg::GpuMemory::freeBuffer(m_icoVbo);
    }
    if (m_icoIbo) {
        vg::GpuMemory::freeBuffer(m_icoIbo);
    }
}

void AtmosphereComponentRenderer::draw(const AtmosphereComponent& aCmp,
                                       const Camera* camera,
                                       const f32v3& relCamPos,
                                       const f32v3& lightDir,
                                       const SpaceLightComponent* spComponent,
                                       f32 planetRadius) {
    // Lazily construct buffer and shaders
    if (!m_program) {
        buildShaders();
        buildMesh();
    }

    m_program->use();
    m_program->enableVertexAttribArrays();

    // Set up matrix
    f32m4 WVP(1.0);
    setMatrixScale(WVP, f32v3(aCmp.radius));
    setMatrixTranslation(WVP, -relCamPos);
    WVP = camera->getViewProjectionMatrix() * WVP;

    static const f32v3 invWavelength(1.0f / 0.65f, 1.0f / 0.57f, 1.0f / 0.475f);
    static const float KR = 0.0025f;
    static const float KM = 0.0020f;
    static const float ESUN = 25.0f;
    static const float KR_ESUN = KR * ESUN;
    static const float KM_ESUN = KM * ESUN;
    static const float KR_4PI = KR * 4.0f * M_PI;
    static const float KM_4PI = KM * 4.0f * M_PI;
    static const float G = 0.99;
    static const float SCALE_DEPTH = 25.0f;

    f32 camHeight2 = relCamPos.x * relCamPos.x + relCamPos.y * relCamPos.y + relCamPos.z * relCamPos.z;

    // Upload uniforms
    glUniformMatrix4fv(m_program->getUniform("unWVP"), 1, GL_FALSE, &WVP[0][0]);
    glUniform3fv(m_program->getUniform("unCameraPos"), 1, &relCamPos[0]);
    glUniform3fv(m_program->getUniform("unLightPos"), 1, &lightDir[0]);
    glUniform3fv(m_program->getUniform("unInvWavelength"), 1, &invWavelength[0]);
    glUniform1f(m_program->getUniform("unCameraHeight2"), camHeight2);
    glUniform1f(m_program->getUniform("unOuterRadius"), aCmp.radius);
    glUniform1f(m_program->getUniform("unOuterRadius2"), aCmp.radius * aCmp.radius);
    glUniform1f(m_program->getUniform("unInnerRadius"), planetRadius);
    glUniform1f(m_program->getUniform("unInnerRadius2"), planetRadius * planetRadius);
    glUniform1f(m_program->getUniform("unKrESun"), KR_ESUN);
    glUniform1f(m_program->getUniform("unKmESun"), KM_ESUN);
    glUniform1f(m_program->getUniform("unKr4PI"), KR_4PI);
    glUniform1f(m_program->getUniform("unKm4PI"), KM_4PI);
    float scale = 1.0f / (aCmp.radius - planetRadius);
    glUniform1f(m_program->getUniform("unScale"), scale);
    glUniform1f(m_program->getUniform("unScaleDepth"), SCALE_DEPTH);
    glUniform1f(m_program->getUniform("unScaleOverScaleDepth"), scale / SCALE_DEPTH);
    glUniform1i(m_program->getUniform("unNumSamples"), 3);
    glUniform1f(m_program->getUniform("unNumSamplesF"), 3.0f);
    glUniform1f(m_program->getUniform("unG"), G);
    glUniform1f(m_program->getUniform("unG"), G * G);

    // Bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, m_icoVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_icoIbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // Render
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    m_program->disableVertexAttribArrays();
    m_program->unuse();
}

void AtmosphereComponentRenderer::buildShaders() {
    // Attributes for spherical terrain
    std::vector<nString> attribs;
    attribs.push_back("vPosition");

    vio::IOManager iom;
    nString vertSource;
    nString fragSource;
    // Build terrain shader
    iom.readFileToString("Shaders/SphericalTerrain/Sky.vert", vertSource);
    iom.readFileToString("Shaders/SphericalTerrain/Sky.frag", fragSource);
    m_program = new vg::GLProgram(true);
    m_program->addShader(vg::ShaderType::VERTEX_SHADER, vertSource.c_str());
    m_program->addShader(vg::ShaderType::FRAGMENT_SHADER, fragSource.c_str());
    m_program->setAttributes(attribs);
    m_program->link();
    m_program->initAttributes();
    m_program->initUniforms();
    // Set constant uniforms
    m_program->use();
    //glUniform1i(m_farTerrainProgram->getUniform("unNormalMap"), 0);
    
    m_program->unuse();
}

void AtmosphereComponentRenderer::buildMesh() {
    std::vector<ui32> indices;
    std::vector<f32v3> positions;

    // TODO(Ben): Optimize with LOD for far viewing
    vmesh::generateIcosphereMesh(ICOSPHERE_SUBDIVISIONS, indices, positions);
    m_numIndices = indices.size();

    vg::GpuMemory::createBuffer(m_icoVbo);
    vg::GpuMemory::createBuffer(m_icoIbo);

    vg::GpuMemory::bindBuffer(m_icoVbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_icoVbo, vg::BufferTarget::ARRAY_BUFFER, positions.size() * sizeof(f32v3),
                                    positions.data(), vg::BufferUsageHint::STATIC_DRAW);
    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ARRAY_BUFFER);

    vg::GpuMemory::bindBuffer(m_icoIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_icoIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ui32),
                                    indices.data(), vg::BufferUsageHint::STATIC_DRAW);
    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
}