#include "stdafx.h"
#include "GasGiantComponentRenderer.h"

#include "ShaderLoader.h"
#include "SpaceSystemComponents.h"
#include "RenderUtils.h"
#include <Vorb/MeshGenerators.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/RasterizerState.h>
#include <Vorb/graphics/ShaderManager.h>

#define ICOSPHERE_SUBDIVISIONS 5

GasGiantComponentRenderer::GasGiantComponentRenderer() {
    // Empty
}

GasGiantComponentRenderer::~GasGiantComponentRenderer() {
    disposeShader();
    if (m_vbo) {
        vg::GpuMemory::freeBuffer(m_vbo);
    }
    if (m_ibo) {
        vg::GpuMemory::freeBuffer(m_ibo);
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
    }
}

void GasGiantComponentRenderer::draw(const GasGiantComponent& ggCmp,
                                     const f32m4& VP,
                                     const f32v3& relCamPos,
                                     const f32v3& lightDir,
                                     const SpaceLightComponent* spCmp,
                                     const AtmosphereComponent* aCmp) {
    // Lazily construct buffer and shaders
    if (!m_program) buildShader();
    if (!m_vbo) buildMesh();

    m_program->use();

    // Set up matrix
    f32m4 WVP(1.0);
    setMatrixScale(WVP, f32v3(ggCmp.radius));
    setMatrixTranslation(WVP, -relCamPos);
    WVP = VP * WVP;

    // Upload uniforms
    static float dt = 1.0f;
    dt += 0.001f;
    glUniform1f(unDT, dt);
    glUniformMatrix4fv(unWVP, 1, GL_FALSE, &WVP[0][0]);
    // Scattering uniforms
    f32 camHeight = glm::length(relCamPos);
    glUniformMatrix4fv(m_program->getUniform("unWVP"), 1, GL_FALSE, &WVP[0][0]);
    glUniform3fv(m_program->getUniform("unCameraPos"), 1, &relCamPos[0]);
    glUniform3fv(m_program->getUniform("unLightDirWorld"), 1, &lightDir[0]);
    glUniform3fv(m_program->getUniform("unInvWavelength"), 1, &aCmp->invWavelength4[0]);
    glUniform1f(m_program->getUniform("unCameraHeight2"), camHeight * camHeight);
    glUniform1f(m_program->getUniform("unOuterRadius"), aCmp->radius);
    glUniform1f(m_program->getUniform("unOuterRadius2"), aCmp->radius * aCmp->radius);
    glUniform1f(m_program->getUniform("unInnerRadius"), aCmp->planetRadius);
    glUniform1f(m_program->getUniform("unKrESun"), aCmp->kr * aCmp->esun);
    glUniform1f(m_program->getUniform("unKmESun"), aCmp->km * aCmp->esun);
    glUniform1f(m_program->getUniform("unKr4PI"), aCmp->kr * M_4_PI);
    glUniform1f(m_program->getUniform("unKm4PI"), aCmp->km * M_4_PI);
    float scale = 1.0f / (aCmp->radius - aCmp->planetRadius);
    glUniform1f(m_program->getUniform("unScale"), scale);
    glUniform1f(m_program->getUniform("unScaleDepth"), aCmp->scaleDepth);
    glUniform1f(m_program->getUniform("unScaleOverScaleDepth"), scale / aCmp->scaleDepth);
    glUniform1i(m_program->getUniform("unNumSamples"), 3);
    glUniform1f(m_program->getUniform("unNumSamplesF"), 3.0f);
    glUniform1f(m_program->getUniform("unG"), aCmp->g);
    glUniform1f(m_program->getUniform("unG2"), aCmp->g * aCmp->g);
   
    // Bind VAO
    glBindVertexArray(m_vao);
   
    // Bind lookup texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ggCmp.colorMap);

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    m_program->unuse();
}

void GasGiantComponentRenderer::disposeShader() {
    if (m_program) {
        vg::ShaderManager::destroyProgram(&m_program);
    }
    // Dispose buffers too for proper reload
    if (m_vbo) {
        vg::GpuMemory::freeBuffer(m_vbo);
    }
    if (m_ibo) {
        vg::GpuMemory::freeBuffer(m_ibo);
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}

void GasGiantComponentRenderer::buildShader() {
    m_program = ShaderLoader::createProgramFromFile("Shaders/GasGiant/GasGiant.vert",
                                                    "Shaders/GasGiant/GasGiant.frag");
    m_program->use();
    glUniform1i(m_program->getUniform("unColorBandLookup"), 0);
    unWVP = m_program->getUniform("unWVP");
    unDT = m_program->getUniform("unDT");
    m_program->unuse();
}

void GasGiantComponentRenderer::buildMesh() {
    std::vector<ui32> indices;
    std::vector<f32v3> positions;

    // TODO(Ben): Optimize with LOD for far viewing
    vmesh::generateIcosphereMesh(ICOSPHERE_SUBDIVISIONS, indices, positions);
    m_numIndices = indices.size();

    std::vector<GasGiantVertex> vertices(positions.size());
    for (int i = 0; i < positions.size(); i++) {
        vertices[i].position = positions[i];
        vertices[i].uv.x = 0.5f;
        vertices[i].uv.y = (positions[i].y + 1.0f) / 2.0f;
    }

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    vg::GpuMemory::createBuffer(m_vbo);
    vg::GpuMemory::createBuffer(m_ibo);

    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_vbo, vg::BufferTarget::ARRAY_BUFFER, vertices.size() * sizeof(GasGiantVertex),
                                    vertices.data(), vg::BufferUsageHint::STATIC_DRAW);

    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ui32),
                                    indices.data(), vg::BufferUsageHint::STATIC_DRAW);

    m_program->enableVertexAttribArrays();
    glVertexAttribPointer(m_program->getAttribute("vPosition"), 3, GL_FLOAT, GL_FALSE, sizeof(GasGiantVertex), offsetptr(GasGiantVertex, position));
    glVertexAttribPointer(m_program->getAttribute("vUV"), 2, GL_FLOAT, GL_FALSE, sizeof(GasGiantVertex), offsetptr(GasGiantVertex, uv));

    glBindVertexArray(0);
}
