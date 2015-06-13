#include "stdafx.h"
#include "VoxelModelRenderer.h"

#include <glm\gtx\transform.hpp>
#include <glm\gtx\euler_angles.hpp>

#include "VoxelMatrix.h"
#include "VoxelModel.h"
#include "VoxelModelMesh.h"

VoxelModelRenderer::VoxelModelRenderer() { /* Empty */ }


VoxelModelRenderer::~VoxelModelRenderer() { /* Empty */ }

void VoxelModelRenderer::draw(VoxelModel* model, f32m4 projectionMatrix, f32v3 translation, f32v3 eulerRotation, f32v3 scale) {
    f32m4 mWVP = projectionMatrix * glm::translate(translation) * glm::eulerAngleX(eulerRotation.x) * glm::eulerAngleY(eulerRotation.y) * glm::eulerAngleZ(eulerRotation.z) * glm::scale(scale);


    glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, false, (f32*)&mWVP[0][0]);

    model->getMesh().bind();
    glVertexAttribPointer(m_program.getAttribute("vPosition"), 3, GL_FLOAT, false, sizeof(VoxelModelVertex), offsetptr(VoxelModelVertex, pos));
    glVertexAttribPointer(m_program.getAttribute("vColor"), 3, GL_UNSIGNED_BYTE, true, sizeof(VoxelModelVertex), offsetptr(VoxelModelVertex, color));
    glVertexAttribPointer(m_program.getAttribute("vNormal"), 3, GL_UNSIGNED_BYTE, false, sizeof(VoxelModelVertex), offsetptr(VoxelModelVertex, normal));

    glDrawElements(GL_TRIANGLES, model->getMesh().getIndexCount(), GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_program.disableVertexAttribArrays();
    vg::GLProgram::unuse();
}