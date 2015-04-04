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
}

void GasGiantComponentRenderer::draw(const GasGiantComponent& ggCmp,
                                     const f32m4& VP,
                                     const f32v3& relCamPos,
                                     const f32v3& lightDir,
                                     const SpaceLightComponent* spComponent) {
    // Lazily construct buffer and shaders
    if (!m_program) buildShader();
    if (!m_vbo) buildMesh();

    m_program->use();
    m_program->enableVertexAttribArrays();

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
    glUniform3fv(unLightDir, 1, &lightDir[0]);
   
    // Bind buffers
    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GasGiantVertex), offsetptr(GasGiantVertex, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GasGiantVertex), offsetptr(GasGiantVertex, uv));

    // Bind lookup texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ggCmp.colorMap);

    vg::RasterizerState::CULL_COUNTER_CLOCKWISE.set();
    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);
    vg::RasterizerState::CULL_CLOCKWISE.set();

    m_program->disableVertexAttribArrays();
    m_program->unuse();
}

void GasGiantComponentRenderer::disposeShader() {
    if (m_program) {
        vg::ShaderManager::destroyProgram(&m_program);
    }
}

void GasGiantComponentRenderer::buildShader() {
    m_program = ShaderLoader::createProgramFromFile("Shaders/GasGiant/GasGiant.vert",
                                                    "Shaders/GasGiant/GasGiant.frag");
    m_program->use();
    glUniform1i(m_program->getUniform("unColorBandLookup"), 0);
    unWVP = m_program->getUniform("unWVP");
    unDT = m_program->getUniform("unDT");
    unLightDir = m_program->getUniform("unLightDir");
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

    vg::GpuMemory::createBuffer(m_vbo);
    vg::GpuMemory::createBuffer(m_ibo);

    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_vbo, vg::BufferTarget::ARRAY_BUFFER, vertices.size() * sizeof(GasGiantVertex),
                                    vertices.data(), vg::BufferUsageHint::STATIC_DRAW);
    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ARRAY_BUFFER);

    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ui32),
                                    indices.data(), vg::BufferUsageHint::STATIC_DRAW);
    vg::GpuMemory::bindBuffer(0, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
}
