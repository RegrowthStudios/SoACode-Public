#include "stdafx.h"
#include "StarComponentRenderer.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include "ShaderLoader.h"
#include "SpaceSystemComponents.h"
#include "RenderUtils.h"

#include <Vorb/MeshGenerators.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/RasterizerState.h>
#include <Vorb/graphics/ShaderManager.h>


#define ICOSPHERE_SUBDIVISIONS 5

StarComponentRenderer::StarComponentRenderer() {
    // Empty
}

StarComponentRenderer::~StarComponentRenderer() {
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

void StarComponentRenderer::draw(const StarComponent& sCmp, const f32m4& VP, const f64q& orientation, const f32v3& relCamPos) {
    // Lazily construct buffer and shaders
    if (!m_program) buildShader();
    if (!m_vbo) buildMesh();

    m_program->use();

    // Convert f64q to f32q
    f32q orientationF32;
    orientationF32.x = (f32)orientation.x;
    orientationF32.y = (f32)orientation.y;
    orientationF32.z = (f32)orientation.z;
    orientationF32.w = (f32)orientation.w;

    // Convert to matrix
    f32m4 rotationMatrix = glm::toMat4(orientationF32);

    // Set up matrix
    f32m4 WVP(1.0);
    setMatrixTranslation(WVP, -relCamPos);
    WVP = VP * WVP * glm::scale(f32v3(sCmp.radius)) * rotationMatrix;

    f32v3 rotRelCamPos = relCamPos * orientationF32;

    // Upload uniforms
    static float dt = 1.0f;
    dt += 0.001f;
    glUniform1f(unDT, dt);
    glUniformMatrix4fv(unWVP, 1, GL_FALSE, &WVP[0][0]);
  
    // Bind VAO
    glBindVertexArray(m_vao);

    glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    m_program->unuse();
}

void StarComponentRenderer::disposeShader() {
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

void StarComponentRenderer::buildShader() {
    m_program = ShaderLoader::createProgramFromFile("Shaders/Star/Star.vert",
                                                    "Shaders/Star/Star.frag");
    m_program->use();
    unWVP = m_program->getUniform("unWVP");
    unDT = m_program->getUniform("unDT");
    m_program->unuse();
}

void StarComponentRenderer::buildMesh() {
    std::vector<ui32> indices;
    std::vector<f32v3> positions;

    // TODO(Ben): Optimize with LOD for far viewing
    vmesh::generateIcosphereMesh(ICOSPHERE_SUBDIVISIONS, indices, positions);
    m_numIndices = indices.size();

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    vg::GpuMemory::createBuffer(m_vbo);
    vg::GpuMemory::createBuffer(m_ibo);

    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_vbo, vg::BufferTarget::ARRAY_BUFFER, positions.size() * sizeof(f32v3),
                                    positions.data(), vg::BufferUsageHint::STATIC_DRAW);

    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(ui32),
                                    indices.data(), vg::BufferUsageHint::STATIC_DRAW);

    m_program->enableVertexAttribArrays();
    glVertexAttribPointer(m_program->getAttribute("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, 0);
   
    glBindVertexArray(0);
}
