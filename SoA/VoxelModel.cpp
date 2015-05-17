#include "stdafx.h"
#include "VoxelModel.h"

#include "VoxelMatrix.h"
#include "VoxelModelLoader.h"

#include <glm/gtx/transform.hpp>

class VoxelModelVertex {
public:
    VoxelModelVertex(f32v3 pos, color3 color):
        pos(pos),
        color(color) {}
    f32v3 pos;
    color3 color;
};

f32v3 VOXEL_MODEL[24] = {
    f32v3(0, 1, 0),
    f32v3(0, 1, 1),
    f32v3(0, 0, 0),
    f32v3(0, 0, 1),

    f32v3(1, 1, 1),
    f32v3(1, 1, 0),
    f32v3(1, 0, 1),
    f32v3(1, 0, 0),

    f32v3(0, 0, 1),
    f32v3(1, 0, 1),
    f32v3(0, 0, 0),
    f32v3(1, 0, 0),

    f32v3(0, 1, 0),
    f32v3(1, 1, 0),
    f32v3(0, 1, 1),
    f32v3(1, 1, 1),

    f32v3(1, 1, 0),
    f32v3(0, 1, 0),
    f32v3(1, 0, 0),
    f32v3(0, 0, 0),

    f32v3(0, 1, 1),
    f32v3(1, 1, 1),
    f32v3(0, 0, 1),
    f32v3(1, 0, 1)
};

ui32 VOXEL_INDICES[6] = {
    0, 2, 1,
    1, 2, 3
};

VoxelModel::VoxelModel():
m_matrices(),
m_verts(0),
m_inds(0),
m_indCount(0),
m_translation(0,0,0),
m_rotation(f32v3(0,0,0)),
m_scale(f32v3(1.0f, 1.0f,1.0f))
{}

VoxelModel::~VoxelModel() {
    for(int i = m_matrices.size() - 1; i >= 0; i--) {
        delete[] m_matrices.back();
        m_matrices.pop_back();
    }
}

void VoxelModel::generateMesh() {
    if(m_verts != 0) glDeleteBuffers(1, &m_verts);
    if(m_inds != 0) glDeleteBuffers(1, &m_inds);

    std::vector<VoxelModelVertex> vertices;
    std::vector<ui32> indices;
    for(VoxelMatrix* matrix : m_matrices) {
        genMatrixMesh(matrix, vertices, indices);
    }

    glGenBuffers(1, &m_verts);
    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VoxelModelVertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_indCount = indices.size();
    glGenBuffers(1, &m_inds);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_inds);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indCount * sizeof(ui32), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void VoxelModel::draw(f32m4 projectionMatrix) {
    m_program.use();
    f32m4 mWVP = projectionMatrix * glm::translate(m_translation) * glm::mat4_cast(m_rotation) * glm::scale(m_scale);

    m_program.use();
    m_program.enableVertexAttribArrays();

    glUniformMatrix4fv(m_program.getUniform("unWVP"), 1, false, (f32*)&mWVP[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, m_verts);
    glVertexAttribPointer(m_program.getAttribute("vPosition"), 3, GL_FLOAT, false, sizeof(VoxelModelVertex), offsetptr(VoxelModelVertex, pos));
    glVertexAttribPointer(m_program.getAttribute("vColor"), 3, GL_UNSIGNED_BYTE, true, sizeof(VoxelModelVertex), offsetptr(VoxelModelVertex, color));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_inds);
    glDrawElements(GL_TRIANGLES, m_indCount, GL_UNSIGNED_INT, nullptr);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    m_program.disableVertexAttribArrays();
    vg::GLProgram::unuse();
}

void VoxelModel::loadFromFile(const nString& path) {
    addMatrices(VoxelModelLoader::loadModel(path));
    generateMesh();
}

void VoxelModel::addMatrix(VoxelMatrix* matrix) {
    m_matrices.push_back(matrix);
}

void VoxelModel::addMatrices(std::vector<VoxelMatrix*> matrices) {
    for(VoxelMatrix* matrix : matrices) {
        m_matrices.push_back(matrix);
    }
}

void VoxelModel::genMatrixMesh(const VoxelMatrix* matrix, std::vector<VoxelModelVertex>& vertices, std::vector<ui32>& indices) {
    f32v3 position = f32v3(matrix->position.x, matrix->position.y, matrix->position.z);
    for(i32 i = 0; i < matrix->size.x; i++) {
        for(i32 j = 0; j < matrix->size.y; j++) {
            for(i32 k = 0; k < matrix->size.z; k++) {
                ColorRGBA8 voxel = matrix->getColor(i, j, k);
                if(voxel.a == 0) continue;
                f32v3 offset = f32v3(i, j, k) + position;
                ColorRGBA8 temp = matrix->getColor(i - 1, j, k);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
                temp = matrix->getColor(i + 1, j, k);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[4 + l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
                temp = matrix->getColor(i, j - 1, k);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[8 + l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
                temp = matrix->getColor(i, j + 1, k);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[12 + l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
                temp = matrix->getColor(i, j, k - 1);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[16 + l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
                temp = matrix->getColor(i, j, k + 1);
                if(temp.a == 0) {
                    i32 indexStart = vertices.size();
                    for(int l = 0; l < 4; l++)
                        vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[20 + l], voxel.rgb));
                    for(int l = 0; l < 6; l++)
                        indices.push_back(indexStart + VOXEL_INDICES[l]);
                }
            }
        }
    }
}