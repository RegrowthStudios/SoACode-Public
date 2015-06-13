#include "stdafx.h"
#include "ModelMesher.h"

#include "VoxelMatrix.h"
#include "VoxelModel.h"
#include "VoxelModelMesh.h"

#include <vector>

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

i32v3 VOXEL_SIDES[6] = {
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
    for(VoxelMatrix* matrix : model->getMatrices()) {
        genMatrixMesh(matrix, vertices, indices);
    }

    if(indices.size() == 0) return VoxelModelMesh(NULL, NULL, 0);

    VGVertexBuffer vbo;
    VGIndexBuffer ibo;
    ui32 indCount;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VoxelModelVertex), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    indCount = indices.size();
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indCount * sizeof(ui32), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return VoxelModelMesh(vbo, ibo, indCount);
}

void ModelMesher::genMatrixMesh(const VoxelMatrix* matrix, std::vector<VoxelModelVertex>& vertices, std::vector<ui32>& indices) {
    f32v3 position = f32v3(matrix->position.x, matrix->position.y, matrix->position.z); // Position of this matrix in it's model model

    for(i32 i = 0; i < matrix->size.x; i++) {
        for(i32 j = 0; j < matrix->size.y; j++) {
            for(i32 k = 0; k < matrix->size.z; k++) {
                ColorRGBA8 voxel = matrix->getColor(i, j, k); // Get the current voxel's color
                if(voxel.a == 0) continue; // If the current voxel is invisible go to next voxel

                f32v3 offset = f32v3(i, j, k) + position; // Position of the current voxel in the model
                for(i32 face = 0; face < 6; face++) { // For each face of the voxel
                    if(matrix->getColor(i32v3(i, j, k) + VOXEL_SIDES[face]).a == 0) { // Check if the adjacent voxel is invisible
                        i32 indexStart = vertices.size(); // Get the position of the first vertex for this face
                        for(int l = 0; l < 4; l++) // Add the 4 vertices for this face
                            vertices.push_back(VoxelModelVertex(offset + VOXEL_MODEL[face * 4 + l], voxel.rgb, f32v3(VOXEL_SIDES[face])));
                        for(int l = 0; l < 6; l++) // Add the 6 indices for this face
                            indices.push_back(indexStart + VOXEL_INDICES[l]);
                    }
                }
            }
        }
    }
}