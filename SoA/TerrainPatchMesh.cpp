#include "stdafx.h"
#include "TerrainPatchMesh.h"

#include <Vorb/TextureRecycler.hpp>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>

#include "Camera.h"
#include "RenderUtils.h"
#include "soaUtils.h"
#include "VoxelCoordinateSpaces.h"
#include "VoxelSpaceConversions.h"

TerrainPatchMesh::~TerrainPatchMesh() {
    if (m_vbo) {
        vg::GpuMemory::freeBuffer(m_vbo);
    }
    if (m_wvbo) {
        vg::GpuMemory::freeBuffer(m_wvbo);
    }
    if (m_wibo) {
        vg::GpuMemory::freeBuffer(m_wibo);
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (m_normalMap) {
        glDeleteTextures(1, &m_normalMap);
    }
}

void TerrainPatchMesh::recycleNormalMap(vg::TextureRecycler* recycler) {
    if (m_normalMap) {
        recycler->recycle(m_normalMap);
        m_normalMap = 0;
    }
}

void TerrainPatchMesh::draw(const f64v3& relativePos, const Camera* camera,
                                const f32m4& rot, vg::GLProgram* program) const {
    // Set up matrix
    f32m4 W(1.0);
    setMatrixTranslation(W, -relativePos);

    f32m4 WVP = camera->getViewProjectionMatrix() * W * rot;
    W *= rot;

    glUniformMatrix4fv(program->getUniform("unWVP"), 1, GL_FALSE, &WVP[0][0]);
    glUniformMatrix4fv(program->getUniform("unW"), 1, GL_FALSE, &W[0][0]);

    // TODO: Using a VAO makes it not work??
    // glBindVertexArray(m_vao);

    glBindTexture(GL_TEXTURE_2D, m_normalMap);

    vg::GpuMemory::bindBuffer(m_vbo, vg::BufferTarget::ARRAY_BUFFER);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, tangent));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, texCoords));
    glVertexAttribPointer(3, 3, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, color));
    glVertexAttribPointer(4, 2, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, normTexCoords));
    glVertexAttribPointer(5, 2, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(TerrainVertex),
                          offsetptr(TerrainVertex, temperature));

    vg::GpuMemory::bindBuffer(m_ibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    glDrawElements(GL_TRIANGLES, PATCH_INDICES, GL_UNSIGNED_SHORT, 0);
    //   glBindVertexArray(0);
}

void TerrainPatchMesh::drawWater(const f64v3& relativePos, const Camera* camera,
                                     const f32m4& rot, vg::GLProgram* program) const {
    // Set up matrix
    f32m4 W(1.0);
    setMatrixTranslation(W, -relativePos);
    W *= rot;
    f32m4 WVP = camera->getViewProjectionMatrix() * W;

    glUniformMatrix4fv(program->getUniform("unWVP"), 1, GL_FALSE, &WVP[0][0]);
    glUniformMatrix4fv(program->getUniform("unW"), 1, GL_FALSE, &W[0][0]);

    // TODO: Using a VAO makes it not work??
    // glBindVertexArray(m_vao);

    vg::GpuMemory::bindBuffer(m_wvbo, vg::BufferTarget::ARRAY_BUFFER);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(WaterVertex),
                          offsetptr(WaterVertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(WaterVertex),
                          offsetptr(WaterVertex, tangent));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(WaterVertex),
                          offsetptr(WaterVertex, color));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE,
                          sizeof(WaterVertex),
                          offsetptr(WaterVertex, texCoords));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE,
                          sizeof(WaterVertex),
                          offsetptr(WaterVertex, depth));


    vg::GpuMemory::bindBuffer(m_wibo, vg::BufferTarget::ELEMENT_ARRAY_BUFFER);
    glDrawElements(GL_TRIANGLES, m_waterIndexCount, GL_UNSIGNED_SHORT, 0);

    //   glBindVertexArray(0);
}

f32v3 TerrainPatchMesh::getClosestPoint(const f32v3& camPos) const {
    return getClosestPointOnAABB(camPos, m_aabbPos, m_aabbDims);
}
f64v3 TerrainPatchMesh::getClosestPoint(const f64v3& camPos) const {
    return getClosestPointOnAABB(camPos, f64v3(m_aabbPos), f64v3(m_aabbDims));
}
