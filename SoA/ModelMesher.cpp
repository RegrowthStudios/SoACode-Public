#include "stdafx.h"
#include "ModelMesher.h"

#include "VoxelMatrix.h"
#include "VoxelModel.h"
#include "VoxelModelMesh.h"

#include <vector>

const f32v3 VOXEL_MODEL[24] = {
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

const ui32 VOXEL_INDICES[6] = {
    0, 2, 1,
    1, 2, 3
};

const i32v3 VOXEL_SIDES[6] = {
    i32v3(-1, 0, 0),
    i32v3(1, 0, 0),
    i32v3(0, -1, 0),
    i32v3(0, 1, 0),
    i32v3(0, 0, -1),
    i32v3(0, 0, 1),
};

VoxelModelMesh ModelMesher::createMesh(const VoxelModel* model) {
    std::vector<VoxelModelVertex> vertices;
    std::vector<ui32> indices;
    VoxelModelMesh rv;
    
    genMatrixMesh(model->getMatrix(), vertices, indices);

    if (indices.size() == 0) return rv;

    glGenVertexArrays(1, &rv.m_vao);
    glBindVertexArray(rv.m_vao);

    glGenBuffers(1, &rv.m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rv.m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VoxelModelVertex), vertices.data(), GL_STATIC_DRAW);
    
    rv.m_indCount = indices.size();
    glGenBuffers(1, &rv.m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rv.m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rv.m_indCount * sizeof(ui32), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(rv.m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, rv.m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rv.m_ibo);
    glBindVertexArray(0);
    // THIS CAUSES CRASH v v v
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return rv;
}

void ModelMesher::genMatrixMesh(const VoxelMatrix& matrix, std::vector<VoxelModelVertex>& vertices, std::vector<ui32>& indices) {
    // TODO(Ben): Could be optimized
    for(i32 i = 0; i < matrix.size.x; i++) {
        for(i32 j = 0; j < matrix.size.y; j++) {
            for(i32 k = 0; k < matrix.size.z; k++) {
                ColorRGBA8 voxel = matrix.getColor(i, j, k); // Get the current voxel's color
                if(voxel.a == 0) continue; // If the current voxel is invisible go to next voxel

                f32v3 offset = f32v3(i, j, k); // Position of the current voxel in the model
                for(i32 face = 0; face < 6; face++) { // For each face of the voxel
                    if(matrix.getColor(i32v3(i, j, k) + VOXEL_SIDES[face]).a == 0) { // Check if the adjacent voxel is invisible
                        i32 indexStart = vertices.size(); // Get the position of the first vertex for this face
                        for(int l = 0; l < 4; l++) // Add the 4 vertices for this face
                            vertices.emplace_back(offset + VOXEL_MODEL[face * 4 + l], voxel.rgb, f32v3(VOXEL_SIDES[face]));
                        for(int l = 0; l < 6; l++) // Add the 6 indices for this face
                            indices.push_back(indexStart + VOXEL_INDICES[l]);
                    }
                }
            }
        }
    }
}