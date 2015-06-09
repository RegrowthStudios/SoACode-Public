#include "stdafx.h"
#include "AtmosphereComponentRenderer.h"

#include "SpaceSystem.h"
#include "RenderUtils.h"
#include "soaUtils.h"
#include "ShaderLoader.h"

#include <Vorb/MeshGenerators.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/RasterizerState.h>
#include <Vorb/graphics/ShaderManager.h>

#define ICOSPHERE_SUBDIVISIONS 5

AtmosphereComponentRenderer::~AtmosphereComponentRenderer() {
    dispose();
}

void AtmosphereComponentRenderer::initGL() {
    if (!m_program.isCreated()) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/AtmosphereShading/Sky.vert",
                                                        "Shaders/AtmosphereShading/Sky.frag");
    }
    if (!m_icoVbo) buildMesh();
}

void AtmosphereComponentRenderer::draw(const AtmosphereComponent& aCmp,
                                       const f32m4& VP,
                                       const f32v3& relCamPos,
                                       const f32v3& lightDir,
                                       const f32 zCoef,
                                       const SpaceLightComponent* spComponent) {
    m_program.use();

    // Set up matrix
    f32m4 WVP(1.0);
    setMatrixScale(WVP, f32v3(aCmp.radius, aCmp.radius * (1.0 - aCmp.oblateness), aCmp.radius));
    setMatrixTranslation(WVP, -relCamPos);
    WVP = VP * WVP;

    f32 camHeight = glm::length(relCamPos);
    f32 camHeight2 = camHeight * camHeight;

    // Upload uniforms
    glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, GL_FALSE, &WVP[0][0]);
    glUniform3fv(m_program.getUniform("unCameraPos"), 1, &relCamPos[0]);
    glUniform3fv(m_program.getUniform("unLightDirWorld"), 1, &lightDir[0]);
    glUniform3fv(m_program.getUniform("unInvWavelength"), 1, &aCmp.invWavelength4[0]);
    glUniform1f(m_program.getUniform("unCameraHeight2"), camHeight2);
    glUniform1f(m_program.getUniform("unOuterRadius"), aCmp.radius);
    glUniform1f(m_program.getUniform("unOuterRadius2"), aCmp.radius * aCmp.radius);
    glUniform1f(m_program.getUniform("unInnerRadius"), aCmp.planetRadius);
    glUniform1f(m_program.getUniform("unKrESun"), aCmp.kr * aCmp.esun);
    glUniform1f(m_program.getUniform("unKmESun"), aCmp.km * aCmp.esun);
    glUniform1f(m_program.getUniform("unKr4PI"), (f32)(aCmp.kr * M_4_PI));
    glUniform1f(m_program.getUniform("unKm4PI"), (f32)(aCmp.km * M_4_PI));
    float scale = 1.0f / (aCmp.radius - aCmp.planetRadius);
    glUniform1f(m_program.getUniform("unScale"), scale);
    glUniform1f(m_program.getUniform("unScaleDepth"), aCmp.scaleDepth);
    glUniform1f(m_program.getUniform("unScaleOverScaleDepth"), scale / aCmp.scaleDepth);
    glUniform1i(m_program.getUniform("unNumSamples"), 3);
    glUniform1f(m_program.getUniform("unNumSamplesF"), 3.0f);
    glUniform1f(m_program.getUniform("unG"), aCmp.g);
    glUniform1f(m_program.getUniform("unG2"), aCmp.g * aCmp.g);
    // For logarithmic Z buffer
    glUniform1f(m_program.getUniform("unZCoef"), zCoef);

    // Bind VAO
    glBindVertexArray(m_vao);
   
    // Render
    glDepthMask(GL_FALSE);
    vg::RasterizerState::CULL_COUNTER_CLOCKWISE.set();
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);
    glDepthMask(GL_TRUE);
    vg::RasterizerState::CULL_CLOCKWISE.set();

    glBindVertexArray(0);
    m_program.unuse(); 
}

void AtmosphereComponentRenderer::dispose() {
    if (m_program.isCreated()) m_program.dispose();
    if (m_icoVbo) vg::GpuMemory::freeBuffer(m_icoVbo);
    if (m_icoIbo) vg::GpuMemory::freeBuffer(m_icoIbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void AtmosphereComponentRenderer::buildMesh() {
    std::vector<ui32> indices;
    std::vector<f32v3> positions;

    // TODO(Ben): Optimize with LOD for far viewing
    vmesh::generateIcosphereMesh(ICOSPHERE_SUBDIVISIONS, indices, positions);
    m_numIndices = indices.size();

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    vg::GpuMemory::createBuffer(m_icoVbo);
    vg::GpuMemory::createBuffer(m_icoIbo);

    vg::GpuMemory::bindBuffer(m_icoVbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_icoVbo, vg::BufferTarget::ARRAY_BUFFER, positions.size() * sizeof(f32v3),
                                    positions.data(), vg::BufferUsageHint::STATIC_DRAW);

    vg::GpuMemory::bindBuffer(m_icoIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_icoIbo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ui32),
                                    indices.data(), vg::BufferUsageHint::STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    m_program.enableVertexAttribArrays();

    glBindVertexArray(0);
}