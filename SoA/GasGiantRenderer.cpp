#include "stdafx.h"
#include "GasGiantRenderer.h"

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include <Vorb\MeshGenerators.h>
#include <Vorb\graphics\GpuMemory.h>
#include <Vorb\BufferUtils.inl>

#include "GLProgramManager.h"

#include <iostream>

void GasGiantMesh::bind() {
    vg::GpuMemory::bindBuffer(this->vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::bindBuffer(this->ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GasGiantVertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GasGiantVertex), (void*)sizeof(f32v3));
}

GasGiantRenderer::GasGiantRenderer(const vg::GLProgramManager* glProgramManager):
    m_glProgramManager(glProgramManager),
    m_shaderProgram(glProgramManager->getProgram("GasGiant")), 
    m_mesh(nullptr)
{
    initMesh();
}

GasGiantRenderer::~GasGiantRenderer() {}

void GasGiantRenderer::drawGasGiant(f32m4& mvp) {
    m_shaderProgram->use();
    m_shaderProgram->enableVertexAttribArrays();
    m_mesh->bind();

    glVertexAttribPointer(m_shaderProgram->getAttribute("vPosition"), 3, GL_FLOAT, GL_FALSE, sizeof(GasGiantVertex), offsetptr(GasGiantVertex, position));
    glVertexAttribPointer(m_shaderProgram->getAttribute("vNormal"), 3, GL_FLOAT, GL_FALSE, sizeof(GasGiantVertex), offsetptr(GasGiantVertex, normal));
    glVertexAttribPointer(m_shaderProgram->getAttribute("vUV"), 2, GL_FLOAT, GL_FALSE, sizeof(GasGiantVertex), offsetptr(GasGiantVertex, uv));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_mesh->colorBandLookup);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glUniform1i(m_mesh->unColorBandLookup, 0);
    
    glUniformMatrix4fv(m_mesh->unWVP, 1, GL_FALSE, (f32*)&mvp[0][0]);
    
    glCullFace(GL_BACK);
    glDrawElements(GL_TRIANGLES, m_mesh->numIndices, GL_UNSIGNED_INT, 0);

    m_shaderProgram->disableVertexAttribArrays();
    m_shaderProgram->unuse();
}

void GasGiantRenderer::initMesh() {
    std::vector<f32v3> positions;
    std::vector<ui32> indices;
    vmesh::generateIcosphereMesh(0, indices, positions);

    std::vector<GasGiantVertex> vertices;
    for(int i = 0; i < positions.size(); i++) {
        GasGiantVertex vertex;
        vertex.position = positions[i];
        vertex.normal = glm::normalize(vertex.position);
        vertex.uv = f32v2(0.5f, (vertex.position.y + 1.0f) / 2.0f);
        vertices.push_back(vertex);
    }

    m_mesh = new GasGiantMesh();
    m_mesh->numIndices = indices.size();
    m_mesh->numVertices = vertices.size();

    vg::GpuMemory::createBuffer(m_mesh->ibo);
    vg::GpuMemory::createBuffer(m_mesh->vbo);

    vg::GpuMemory::bindBuffer(m_mesh->vbo, vg::BufferTarget::ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_mesh->vbo, vg::BufferTarget::ARRAY_BUFFER, vertices.size() * sizeof(GasGiantVertex), vertices.data());

    vg::GpuMemory::bindBuffer(m_mesh->ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    vg::GpuMemory::uploadBufferData(m_mesh->ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data());

    m_mesh->unWVP = m_shaderProgram->getUniform("unWVP");
    m_mesh->unColorBandLookup = m_shaderProgram->getUniform("unColorBandLookup");

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
