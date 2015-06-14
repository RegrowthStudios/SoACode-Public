#include "stdafx.h"
#include "ModelMesher.h"

#include "VoxelMatrix.h"
#include "VoxelModel.h"
#include "VoxelModelMesh.h"
#include "MarchingCubesTable.h"

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
    rv.m_triCount = (indices.size() * 2) / 6;
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

VoxelModelMesh ModelMesher::createMarchingCubesMesh(const VoxelModel* model) {
    std::vector<VoxelModelVertex> vertices;
    std::vector<ui32> indices;
    VoxelModelMesh rv;

    auto& matrix = model->getMatrix();

    f32v4* points = new f32v4[(matrix.size.x + 1) * (matrix.size.y + 1) * (matrix.size.z + 1)];
    int index = 0;
    for (i32 x = 0; x < matrix.size.x + 1; x++) {
        for (i32 y = 0; y < matrix.size.y + 1; y++) {
            for (i32 z = 0; z < matrix.size.z + 1; z++) {
                f32v4 vert(x, y, z, 0);
                vert.w = getMarchingPotential(matrix, x, y, z);
               
                points[x*(matrix.size.y + 1)*(matrix.size.z + 1) + y*(matrix.size.z + 1) + z] = vert;
                index++;
            }
        }
    }

    marchingCubesCross(matrix, 0.5f, points, vertices);

    rv.m_triCount = vertices.size() / 3;
    glGenVertexArrays(1, &rv.m_vao);
    glBindVertexArray(rv.m_vao);

    glGenBuffers(1, &rv.m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, rv.m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VoxelModelVertex), vertices.data(), GL_STATIC_DRAW);

    rv.m_indCount = 0;
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

color3 ModelMesher::getColor(const f32v3& pos, const VoxelMatrix& matrix) {
    i32v3 ipos(glm::round(pos));
    if (ipos.x >= matrix.size.x) ipos.x = matrix.size.x - 1;
    if (ipos.y >= matrix.size.y) ipos.y = matrix.size.y - 1;
    if (ipos.z >= matrix.size.z) ipos.z = matrix.size.z - 1;
    int x = ipos.x;
    int y = ipos.y;
    int z = ipos.z;

    int numColors = 0;
    i32v3 fColor(0);

    if (y > 0) {
        if (z > 0) {
            // Bottom back left
            if (x > 0) {
                color4 color = matrix.getColor(x - 1, y - 1, z - 1);
                if (color.a != 0 && !matrix.isInterior(x - 1, y - 1, z - 1)) {
                    numColors++;
                    fColor.r += color.r;
                    fColor.g += color.g;
                    fColor.b += color.b;
                }
            }
            // Bottom back right
            if (x < matrix.size.x) {
                color4 color = matrix.getColor(x, y - 1, z - 1);
                if (color.a != 0 && !matrix.isInterior(x, y - 1, z - 1)) {
                    numColors++;
                    fColor.r += color.r;
                    fColor.g += color.g;
                    fColor.b += color.b;
                }
            }
        }
        if (z < matrix.size.z) {
            // Bottom front left
            if (x > 0) {
                color4 color = matrix.getColor(x - 1, y - 1, z);
                if (color.a != 0 && !matrix.isInterior(x - 1, y - 1, z)) {
                    numColors++;
                    fColor.r += color.r;
                    fColor.g += color.g;
                    fColor.b += color.b;
                }
            }
            // Bottom front right
            if (x < matrix.size.x) {
                color4 color = matrix.getColor(x, y - 1, z);
                if (color.a != 0 && !matrix.isInterior(x, y - 1, z)) {
                    numColors++;
                    fColor.r += color.r;
                    fColor.g += color.g;
                    fColor.b += color.b;
                }
            }
        }

    }

    if (y < matrix.size.y) {
        if (z > 0) {
            // Top back left
            if (x > 0) {
                color4 color = matrix.getColor(x - 1, y, z - 1);
                if (color.a != 0 && !matrix.isInterior(x - 1, y, z - 1)) {
                    numColors++;
                    fColor.r += color.r;
                    fColor.g += color.g;
                    fColor.b += color.b;
                }
            }
            // Top back right
            if (x < matrix.size.x) {
                color4 color = matrix.getColor(x, y, z - 1);
                if (color.a != 0 && !matrix.isInterior(x, y, z - 1)) {
                    numColors++;
                    fColor.r += color.r;
                    fColor.g += color.g;
                    fColor.b += color.b;
                }
            }
        }
        if (z < matrix.size.z) {
            // Top front left
            if (x > 0) {
                color4 color = matrix.getColor(x - 1, y, z);
                if (color.a != 0 && !matrix.isInterior(x - 1, y, z)) {
                    numColors++;
                    fColor.r += color.r;
                    fColor.g += color.g;
                    fColor.b += color.b;
                }
            }
            // Top front right
            if (x < matrix.size.x) {
                color4 color = matrix.getColor(x, y, z);
                if (color.a != 0 && !matrix.isInterior(x, y, z)) {
                    numColors++;
                    fColor.r += color.r;
                    fColor.g += color.g;
                    fColor.b += color.b;
                }
            }
        }
    }
    if (numColors) {
        fColor /= numColors;
    }

    return color3(fColor.r, fColor.g, fColor.b);
}

//Linear Interpolation function
f32v3 linearInterp(const f32v4& p1, const f32v4& p2, float value) {
    f32v3 p;
    if (p1.w != p2.w)
        p = f32v3(p1) + (f32v3(p2) - f32v3(p1)) / (p2.w - p1.w)*(value - p1.w);
    else
        p = (f32v3)p1;
    return p;
}

void ModelMesher::marchingCubesCross(const VoxelMatrix& matrix,
                                     float minValue, f32v4 * points,
                                     std::vector<VoxelModelVertex>& vertices) {

    int ncellsX = matrix.size.x;
    int ncellsY = matrix.size.y;
    int ncellsZ = matrix.size.z;

    int YtimeZ = (ncellsY + 1)*(ncellsZ + 1);	//for little extra speed
    int ni, nj;
    f32v3 mainOffset(-ncellsX / 2.0f, -ncellsY / 2.0f, -ncellsZ / 2.0f);

    //go through all the points
    for (int i = 0; i < ncellsX; i++) {		//x axis
        ni = i*YtimeZ;
        for (int j = 0; j < ncellsY; j++) {	//y axis
            nj = j*(ncellsZ + 1);
            for (int k = 0; k < ncellsZ; k++)	//z axis
            {
                //initialize vertices
                f32v4 verts[8];
                int ind = ni + nj + k;
                /*(step 3)*/
                verts[0] = points[ind];
                verts[1] = points[ind + YtimeZ];
                verts[2] = points[ind + YtimeZ + 1];
                verts[3] = points[ind + 1];
                verts[4] = points[ind + (ncellsZ + 1)];
                verts[5] = points[ind + YtimeZ + (ncellsZ + 1)];
                verts[6] = points[ind + YtimeZ + (ncellsZ + 1) + 1];
                verts[7] = points[ind + (ncellsZ + 1) + 1];

                //get the index
                int cubeIndex = int(0);
                for (int n = 0; n < 8; n++)
                /*(step 4)*/
                if (verts[n].w <= minValue) cubeIndex |= (1 << n);

                //check if its completely inside or outside
                /*(step 5)*/
                if (!edgeTable[cubeIndex]) continue;

                //get linearly interpolated vertices on edges and save into the array
                f32v3 intVerts[12];
                /*(step 6)*/
                if (edgeTable[cubeIndex] & 1) intVerts[0] = linearInterp(verts[0], verts[1], minValue);
                if (edgeTable[cubeIndex] & 2) intVerts[1] = linearInterp(verts[1], verts[2], minValue);
                if (edgeTable[cubeIndex] & 4) intVerts[2] = linearInterp(verts[2], verts[3], minValue);
                if (edgeTable[cubeIndex] & 8) intVerts[3] = linearInterp(verts[3], verts[0], minValue);
                if (edgeTable[cubeIndex] & 16) intVerts[4] = linearInterp(verts[4], verts[5], minValue);
                if (edgeTable[cubeIndex] & 32) intVerts[5] = linearInterp(verts[5], verts[6], minValue);
                if (edgeTable[cubeIndex] & 64) intVerts[6] = linearInterp(verts[6], verts[7], minValue);
                if (edgeTable[cubeIndex] & 128) intVerts[7] = linearInterp(verts[7], verts[4], minValue);
                if (edgeTable[cubeIndex] & 256) intVerts[8] = linearInterp(verts[0], verts[4], minValue);
                if (edgeTable[cubeIndex] & 512) intVerts[9] = linearInterp(verts[1], verts[5], minValue);
                if (edgeTable[cubeIndex] & 1024) intVerts[10] = linearInterp(verts[2], verts[6], minValue);
                if (edgeTable[cubeIndex] & 2048) intVerts[11] = linearInterp(verts[3], verts[7], minValue);

                //now build the triangles using triTable
                for (int n = 0; triTable[cubeIndex][n] != -1; n += 3) {
                    int startIndex = vertices.size();
                    vertices.resize(vertices.size() + 3);
                    /*(step 7)*/
                    vertices[startIndex].pos = intVerts[triTable[cubeIndex][n + 2]];
                    vertices[startIndex + 1].pos = intVerts[triTable[cubeIndex][n + 1]];
                    vertices[startIndex + 2].pos = intVerts[triTable[cubeIndex][n]];

                    f32v3 p1 = vertices[startIndex + 1].pos - vertices[startIndex].pos;
                    f32v3 p2 = vertices[startIndex + 2].pos - vertices[startIndex].pos;
                    //Computing normal as cross product of triangle's edges
                    /*(step 8)*/
                    f32v3 normal = glm::normalize(glm::cross(p1, p2));
                    vertices[startIndex].normal = normal;
                    vertices[startIndex + 1].normal = normal;
                    vertices[startIndex + 2].normal = normal;
                    // Offsets and color
                    vertices[startIndex].color = getColor(vertices[startIndex].pos, matrix);
                    vertices[startIndex + 1].color = getColor(vertices[startIndex + 1].pos, matrix);
                    vertices[startIndex + 2].color = getColor(vertices[startIndex + 2].pos, matrix);

                    vertices[startIndex].pos += mainOffset;
                    vertices[startIndex + 1].pos += mainOffset;
                    vertices[startIndex + 2].pos += mainOffset;
                }

            }
        }
    }
}

f32 ModelMesher::getMarchingPotential(const VoxelMatrix& matrix, int x, int y, int z) {
    f32 potential = 0.0f;
    int filterSize = 2;
    int halfFilterSize = filterSize / 2;
    x -= halfFilterSize;
    y -= halfFilterSize;
    z -= halfFilterSize;
    for (int i = 0; i < filterSize; i++) {
        for (int j = 0; j < filterSize; j++) {
            for (int k = 0; k < filterSize; k++) {
                if (matrix.getColorAndCheckBounds(x + i, y + j, z + k).a != 0) {
                    potential += 1.0f;
                }
            }
        }
    }
   
    return potential / (filterSize * filterSize * filterSize);
}

void ModelMesher::genMatrixMesh(const VoxelMatrix& matrix, std::vector<VoxelModelVertex>& vertices, std::vector<ui32>& indices) {
    // TODO(Ben): Could be optimized
    f32v3 mainOffset(matrix.size.x / 2.0f, matrix.size.y / 2.0f, matrix.size.z / 2.0f);
    int voxelIndex;
    for(int z = 0; z < matrix.size.z; z++) {
        for (int y = 0; y < matrix.size.y; y++) {
            for (int x = 0; x < matrix.size.x; x++) {
                voxelIndex = matrix.getIndex(x, y, z);
                const ColorRGBA8& voxel = matrix.getColor(voxelIndex); // Get the current voxel's color
                if(voxel.a == 0) continue; // If the current voxel is invisible go to next voxel

                f32v3 offset = f32v3(x, y, z) - mainOffset; // Position of the current voxel in the model
                for (int face = 0; face < 6; face++) { // For each face of the voxel
                    if(matrix.getColorAndCheckBounds(i32v3(x, y, z) + VOXEL_SIDES[face]).a == 0) { // Check if the adjacent voxel is invisible
                        int indexStart = (int)vertices.size();
                        int indiceStart = (int)indices.size();

                        // Add the 4 vertices for this face
                        vertices.resize(indexStart + 4);
                        for (int l = 0; l < 4; l++) {
                            vertices[indexStart + l].pos = offset + VOXEL_MODEL[face * 4 + l];
                            vertices[indexStart + l].color = voxel.rgb;
                            vertices[indexStart + l].normal = f32v3(VOXEL_SIDES[face]);
                        }

                        // Add the 6 indices for this face
                        indices.resize(indiceStart + 6);
                        indices[indiceStart] = indexStart + VOXEL_INDICES[0];
                        indices[indiceStart + 1] = indexStart + VOXEL_INDICES[1];
                        indices[indiceStart + 2] = indexStart + VOXEL_INDICES[2];
                        indices[indiceStart + 3] = indexStart + VOXEL_INDICES[3];
                        indices[indiceStart + 4] = indexStart + VOXEL_INDICES[4];
                        indices[indiceStart + 5] = indexStart + VOXEL_INDICES[5];
                    }
                }
            }
        }
    }
}