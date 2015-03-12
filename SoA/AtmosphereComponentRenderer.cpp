#include "stdafx.h"
#include "AtmosphereComponentRenderer.h"

#include "SpaceSystem.h"
#include "RenderUtils.h"
#include "soaUtils.h"

#include "Camera.h"
#include <Vorb/io/IOManager.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/MeshGenerators.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/RasterizerState.h>

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
                                       const f32m4& VP,
                                       const f32v3& relCamPos,
                                       const f32v3& lightDir,
                                       const SpaceLightComponent* spComponent) {
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
    WVP = VP * WVP;

    f32 camHeight = glm::length(relCamPos);
    f32 camHeight2 = camHeight * camHeight;

    vg::RasterizerState::CULL_COUNTER_CLOCKWISE.set();

    // Upload uniforms
    glUniformMatrix4fv(m_program->getUniform("unWVP"), 1, GL_FALSE, &WVP[0][0]);
    glUniform3fv(m_program->getUniform("unCameraPos"), 1, &relCamPos[0]);
    glUniform3fv(m_program->getUniform("unLightPos"), 1, &lightDir[0]);
    glUniform3fv(m_program->getUniform("unInvWavelength"), 1, &aCmp.invWavelength4[0]);
    glUniform1f(m_program->getUniform("unCameraHeight2"), camHeight2);
    glUniform1f(m_program->getUniform("unOuterRadius"), aCmp.radius);
    glUniform1f(m_program->getUniform("unOuterRadius2"), aCmp.radius * aCmp.radius);
    glUniform1f(m_program->getUniform("unInnerRadius"), aCmp.planetRadius);
    glUniform1f(m_program->getUniform("unKrESun"), aCmp.krEsun);
    glUniform1f(m_program->getUniform("unKmESun"), aCmp.kmEsun);
    glUniform1f(m_program->getUniform("unKr4PI"), aCmp.kr4PI);
    glUniform1f(m_program->getUniform("unKm4PI"), aCmp.km4PI);
    float scale = 1.0f / (aCmp.radius - aCmp.planetRadius);
    glUniform1f(m_program->getUniform("unScale"), scale);
    glUniform1f(m_program->getUniform("unScaleDepth"), aCmp.scaleDepth);
    glUniform1f(m_program->getUniform("unScaleOverScaleDepth"), scale / aCmp.scaleDepth);
    glUniform1i(m_program->getUniform("unNumSamples"), 3);
    glUniform1f(m_program->getUniform("unNumSamplesF"), 3.0f);
    glUniform1f(m_program->getUniform("unG"), aCmp.g);
    glUniform1f(m_program->getUniform("unG2"), aCmp.g * aCmp.g);

    // Bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, m_icoVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_icoIbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // Render
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    m_program->disableVertexAttribArrays();
    m_program->unuse();
    vg::RasterizerState::CULL_CLOCKWISE.set();
}

void AtmosphereComponentRenderer::buildShaders() {
    // Attributes for spherical terrain
    std::vector<nString> attribs;
    attribs.push_back("vPosition");

    vio::IOManager iom;
    nString vertSource;
    nString fragSource;
    // Build terrain shader
    iom.readFileToString("Shaders/AtmosphereShading/Sky.vert", vertSource);
    iom.readFileToString("Shaders/AtmosphereShading/Sky.frag", fragSource);
    m_program = new vg::GLProgram(true);
    m_program->addShader(vg::ShaderType::VERTEX_SHADER, vertSource.c_str());
    m_program->addShader(vg::ShaderType::FRAGMENT_SHADER, fragSource.c_str());
    m_program->setAttributes(attribs);
    m_program->link();
    m_program->initAttributes();
    m_program->initUniforms();
    // Set constant uniforms
   // m_program->use();
   
   // m_program->unuse();
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