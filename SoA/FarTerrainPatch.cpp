#include "stdafx.h"
#include "FarTerrainPatch.h"

#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/TextureRecycler.hpp>
#include <Vorb/utils.h>

#include "Camera.h"
#include "RenderUtils.h"
#include "VoxelCoordinateSpaces.h"
#include "VoxelSpaceConversions.h"
#include "SphericalTerrainPatchMesher.h"

const f32v3 NormalMults[6] = {
    f32v3(1.0f, 1.0f, -1.0f), //TOP
    f32v3(1.0f, 1.0f, -1.0f), //LEFT
    f32v3(1.0f, 1.0f, 1.0f), //RIGHT
    f32v3(1.0f, 1.0f, 1.0f), //FRONT
    f32v3(1.0f, 1.0f, -1.0f), //BACK
    f32v3(1.0f, 1.0f, 1.0f) //BOTTOM
};

FarTerrainMesh::~FarTerrainMesh() {
}

void FarTerrainMesh::recycleNormalMap(vg::TextureRecycler* recycler) {
    if (m_normalMap) {
        recycler->recycle(m_normalMap);
        m_normalMap = 0;
    }
}

void FarTerrainMesh::draw(const f64v3& relativePos, const Camera* camera, vg::GLProgram* program) const {
    // Set up matrix
    f32m4 W(1.0);
    setMatrixTranslation(W, -relativePos);

    f32m4 WVP = camera->getViewProjectionMatrix() * W;

    glUniform3fv(program->getUniform("unNormMult"), 1, &NormalMults[(int)m_cubeFace][0]);
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
    glDrawElements(GL_TRIANGLES, SphericalTerrainPatch::INDICES_PER_PATCH, GL_UNSIGNED_SHORT, 0);
}

void FarTerrainMesh::drawWater(const f64v3& relativePos, const Camera* camera, vg::GLProgram* program) const {

}

void FarTerrainMesh::getClosestPoint(const f32v3& camPos, OUT f32v3& point) const {

}

void FarTerrainMesh::getClosestPoint(const f64v3& camPos, OUT f64v3& point) const {

}

FarTerrainPatch::~FarTerrainPatch() {

}

void FarTerrainPatch::init(const f64v2& gridPosition, WorldCubeFace cubeFace, int lod, const SphericalTerrainData* sphericalTerrainData, f64 width, TerrainRpcDispatcher* dispatcher) {

}

void FarTerrainPatch::update(const f64v3& cameraPos) {

}

void FarTerrainPatch::destroy() {

}

bool FarTerrainPatch::isRenderable() const {

}

bool FarTerrainPatch::isOverHorizon(const f32v3 &relCamPos, const f32v3 &point, f32 planetRadius) {

}

bool FarTerrainPatch::isOverHorizon(const f64v3 &relCamPos, const f64v3 &point, f64 planetRadius) {

}

void FarTerrainPatch::requestMesh() {

}
