#include "stdafx.h"
#include "VoxelModelRenderer.h"
#include "ShaderLoader.h"

#include <glm/gtx/quaternion.hpp>

#include "VoxelMatrix.h"
#include "VoxelModel.h"
#include "VoxelModelMesh.h"
#include "RenderUtils.h"

void VoxelModelRenderer::initGL() {
    m_program = ShaderLoader::createProgramFromFile("Shaders/Models/VoxelModel.vert",
                                                    "Shaders/Models/VoxelModel.frag");
}

void VoxelModelRenderer::dispose() {
    if (m_program.isCreated()) m_program.dispose();
}

void VoxelModelRenderer::draw(VoxelModelMesh* mesh, f32m4 mVP, const f64v3& relativePos, const f64q& orientation) {

    // Convert f64q to f32q
    f32q orientationF32;
    orientationF32.x = (f32)orientation.x;
    orientationF32.y = (f32)orientation.y;
    orientationF32.z = (f32)orientation.z;
    orientationF32.w = (f32)orientation.w;
    // Convert to matrix
    f32m4 rotationMatrix = glm::toMat4(orientationF32);
    f32m4 mW(1.0);
    setMatrixTranslation(mW, -relativePos);
    f32m4 mWVP = mVP * mW * rotationMatrix;

    m_program.use();
    glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, false, &mWVP[0][0]);
   
    // TODO(Ben): Temporary
    f32v3 lightDir = glm::normalize(f32v3(1.0f, 0.0f, 1.0f));
    glUniform3fv(m_program.getUniform("unLightDirWorld"), 1, &lightDir[0]);

    mesh->bind();
    m_program.enableVertexAttribArrays();
    glVertexAttribPointer(m_program.getAttribute("vPosition"), 3, GL_FLOAT, false, sizeof(VoxelModelVertex), offsetptr(VoxelModelVertex, pos));
    glVertexAttribPointer(m_program.getAttribute("vNormal"), 3, GL_FLOAT, false, sizeof(VoxelModelVertex), offsetptr(VoxelModelVertex, normal));
    glVertexAttribPointer(m_program.getAttribute("vColor"), 3, GL_UNSIGNED_BYTE, true, sizeof(VoxelModelVertex), offsetptr(VoxelModelVertex, color));
    
    if (mesh->getIndexCount() == 0) {
        glDrawArrays(GL_TRIANGLES, 0, mesh->getTriCount() * 3);
    } else {
        glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, nullptr);
    }
    mesh->unbind();

    m_program.unuse();
}