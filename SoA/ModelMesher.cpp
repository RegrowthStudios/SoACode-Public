#include "stdafx.h"
#include "ModelMesher.h"

#include "VoxelMatrix.h"
#include "VoxelModel.h"
#include "VoxelModelMesh.h"
#include "MarchingCubesTable.h"
#include "DualContouringMesher.h"

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

    // This constructs a potential (density) field from our voxel data, with points at the corner of each voxel
    f32v4* points = new f32v4[(matrix.size.x + 1) * (matrix.size.y + 1) * (matrix.size.z + 1)];
    int index = 0;
    // + 1 since its the corner points intead of the actual voxels (for smoother mesh)
    for (ui32 x = 0; x < matrix.size.x + 1; x++) {
        for (ui32 y = 0; y < matrix.size.y + 1; y++) {
            for (ui32 z = 0; z < matrix.size.z + 1; z++) {
                f32v4 vert(x, y, z, 0);
                // Gets the potential
                vert.w = getMarchingPotential(matrix, x, y, z);
               
                points[x * (matrix.size.y + 1) * (matrix.size.z + 1) + y * (matrix.size.z + 1) + z] = vert;
                index++;
            }
        }
    }

    marchingCubes(matrix, 1.0f, 1.0f, 1.0f, 0.5f, points, vertices);

    // TODO(Ben): Indexed drawing
    rv.m_triCount = vertices.size() / 3;
    delete[] points;

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

VoxelModelMesh ModelMesher::createDualContouringMesh(const VoxelModel* model) {
    std::vector<VoxelModelVertex> vertices;
    std::vector<ui32> indices;
    VoxelModelMesh rv;

    DualContouringMesher::genMatrixMesh(model->getMatrix(), vertices, indices);

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

// Gets the potential at voxel corners
f32 ModelMesher::getMarchingPotential(const VoxelMatrix& matrix, int x, int y, int z) {

    // TODO: This could all be written from scratch. Not sure the best way to go about it.

    f32 potential = 0.0f;
    const int FILTER_SIZE = 2; ///< Should be at least 2
    const int HALF_FILTER_SIZE = FILTER_SIZE / 2;
    x -= HALF_FILTER_SIZE;
    y -= HALF_FILTER_SIZE;
    z -= HALF_FILTER_SIZE;

    // We use a 2x2x2 filter of voxels around a given voxel corner point
    for (int i = 0; i < FILTER_SIZE; i++) {
        for (int j = 0; j < FILTER_SIZE; j++) {
            for (int k = 0; k < FILTER_SIZE; k++) {
                // TODO: Play with this!

                if (matrix.getColorAndCheckBounds(x + i, y + j, z + k).a != 0) {
                    potential += 1.0f;
                }
                // Interesting effect:
                /* float dx = abs(i - 1.5f);
                 float dy = abs(j - 1.5f);
                 float dz = abs(k - 1.5f);
                 if (matrix.getColorAndCheckBounds(x + i, y + j, z + k).a != 0) {
                 if (dx <= 0.75f && dy <= 0.75f && dz <= 0.75f) {
                 potential += 2.0f;
                 } else {
                 potential += 0.75f;
                 }
                 } else if (dx <= 0.75f && dy <= 0.75f && dz <= 0.75f) {
                 potential -= 2.0f;
                 }*/
            }
        }
    }
    // Average the samples
    return potential / (FILTER_SIZE * FILTER_SIZE * FILTER_SIZE);

    if (matrix.getColor(x, y, z).a != 0.0) {
        potential += 2.0f;
    }

    // Add extra potential from neighbors ( makes it smoother )
    // When .a is 0.0, that is an air block
    if (matrix.getColorAndCheckBounds(x - 1, y, z).a != 0.0) {
        potential += 0.25f;
    }
    if (matrix.getColorAndCheckBounds(x + 1, y, z).a != 0.0) {
        potential += 0.25f;
    }
    if (matrix.getColorAndCheckBounds(x, y - 1, z).a != 0.0) {
        potential += 0.25f;
    }
    if (matrix.getColorAndCheckBounds(x, y + 1, z).a != 0.0) {
        potential += 0.25f;
    }
    if (matrix.getColorAndCheckBounds(x, y, z - 1).a != 0.0) {
        potential += 0.25f;
    }
    if (matrix.getColorAndCheckBounds(x, y, z + 1).a != 0.0) {
        potential += 0.25f;
    }

    // Divide by 7 to average with neighbors a bit
    return potential / 7.0f;
}

// This gets color for marching cubes vertices by averaging nearby voxel colors
// TODO: Figure out a metter method?
color3 ModelMesher::getColor(const f32v3& pos, const VoxelMatrix& matrix) {
    ui32v3 ipos(glm::round(pos));
    if (ipos.x >= matrix.size.x) ipos.x = matrix.size.x - 1;
    if (ipos.y >= matrix.size.y) ipos.y = matrix.size.y - 1;
    if (ipos.z >= matrix.size.z) ipos.z = matrix.size.z - 1;
    ui32 x = ipos.x;
    ui32 y = ipos.y;
    ui32 z = ipos.z;

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

color3 ModelMesher::getColor2(const i32v3& pos, const VoxelMatrix& matrix) {
    int numColors = 1;
    i32v3 fColor(0);

    { // Center
        color4 vColor = matrix.getColorAndCheckBounds(pos);
        if (vColor.a != 0 && !matrix.isInterior(pos)) {
            return vColor.rgb;
        }
    }
    { // Left
        i32v3 vPos = pos + i32v3(-1, 0, 0);
        color4 vColor = matrix.getColorAndCheckBounds(vPos);
        if (vColor.a != 0 && !matrix.isInterior(vPos)) {
            return vColor.rgb;
        }
    }
    { // Right
        i32v3 vPos = pos + i32v3(1, 0, 0);
        color4 vColor = matrix.getColorAndCheckBounds(vPos);
        if (vColor.a != 0 && !matrix.isInterior(vPos)) {
            return vColor.rgb;
        }
    }
    { // Bottom
        i32v3 vPos = pos + i32v3(0, -1, 0);
        color4 vColor = matrix.getColorAndCheckBounds(vPos);
        if (vColor.a != 0 && !matrix.isInterior(vPos)) {
            return vColor.rgb;
        }
    }
    { // Top
        i32v3 vPos = pos + i32v3(0, 1, 0);
        color4 vColor = matrix.getColorAndCheckBounds(vPos);
        if (vColor.a != 0 && !matrix.isInterior(vPos)) {
            return vColor.rgb;
        }
    }
    { // Back
        i32v3 vPos = pos + i32v3(0, 0, -1);
        color4 vColor = matrix.getColorAndCheckBounds(vPos);
        if (vColor.a != 0 && !matrix.isInterior(vPos)) {
            return vColor.rgb;
        }
    }
    { // Front
        i32v3 vPos = pos + i32v3(0, 0, 1);
        color4 vColor = matrix.getColorAndCheckBounds(vPos);
        if (vColor.a != 0 && !matrix.isInterior(vPos)) {
            return vColor.rgb;
        }
    }
    return color3(0, 0, 0);
    if (numColors) {
        fColor /= numColors;
    }

    return color3(fColor.r, fColor.g, fColor.b);
}

//Macros used to compute gradient vector on each vertex of a cube
//argument should be the name of array of vertices
//can be verts or *verts if done by reference
#define CALC_GRAD_VERT_0(verts) f32v4(points[ind-YtimeZ].w-(verts[1]).w,points[ind-pointsZ].w-(verts[4]).w,points[ind-1].w-(verts[3]).w, (verts[0]).w);
#define CALC_GRAD_VERT_1(verts) f32v4((verts[0]).w-points[ind+2*YtimeZ].w,points[ind+YtimeZ-pointsZ].w-(verts[5]).w,points[ind+YtimeZ-1].w-(verts[2]).w, (verts[1]).w);
#define CALC_GRAD_VERT_2(verts) f32v4((verts[3]).w-points[ind+2*YtimeZ+1].w,points[ind+YtimeZ-ncellsZ].w-(verts[6]).w,(verts[1]).w-points[ind+YtimeZ+2].w, (verts[2]).w);
#define CALC_GRAD_VERT_3(verts) f32v4(points[ind-YtimeZ+1].w-(verts[2]).w,points[ind-ncellsZ].w-(verts[7]).w,(verts[0]).w-points[ind+2].w, (verts[3]).w);
#define CALC_GRAD_VERT_4(verts) f32v4(points[ind-YtimeZ+ncellsZ+1].w-(verts[5]).w,(verts[0]).w-points[ind+2*pointsZ].w,points[ind+ncellsZ].w-(verts[7]).w, (verts[4]).w);
#define CALC_GRAD_VERT_5(verts) f32v4((verts[4]).w-points[ind+2*YtimeZ+ncellsZ+1].w,(verts[1]).w-points[ind+YtimeZ+2*pointsZ].w,points[ind+YtimeZ+ncellsZ].w-(verts[6]).w, (verts[5]).w);
#define CALC_GRAD_VERT_6(verts) f32v4((verts[7]).w-points[ind+2*YtimeZ+ncellsZ+2].w,(verts[2]).w-points[ind+YtimeZ+2*ncellsZ+3].w,(verts[5]).w-points[ind+YtimeZ+ncellsZ+3].w, (verts[6]).w);
#define CALC_GRAD_VERT_7(verts) f32v4(points[ind-YtimeZ+ncellsZ+2].w-(verts[6]).w,(verts[3]).w-points[ind+2*ncellsZ+3].w,(verts[4]).w-points[ind+ncellsZ+3].w, (verts[7]).w);

///////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL //
//Global variables - so they dont have to be passed into functions
int pointsZ;	//number of points on Z zxis (equal to ncellsZ+1)
int YtimeZ;		//'plane' of cubes on YZ (equal to (ncellsY+1)*pointsZ )
///////////////////////////////////////////////////////////////////////////////////////////////////////

//Linear Interpolation between two points
f32v3 linearInterp(const f32v4& p1, const f32v4& p2, float value) {
    f32v3 p13(p1);
    if (fabs(p1.w - p2.w) > 0.00001f)
        return p13 + (f32v3(p2) - p13) / (p2.w - p1.w)*(value - p1.w);
    else
        return p13;
}


// Source: http://www.angelfire.com/linux/myp/MC/
// With Improvements: http://www.angelfire.com/linux/myp/MCAdvanced/MCImproved.html
void ModelMesher::marchingCubes(const VoxelMatrix& matrix,
                                float gradFactorX, float gradFactorY, float gradFactorZ,
                                float minValue, f32v4 * points, std::vector<VoxelModelVertex>& vertices) {
    int ncellsX = matrix.size.x;
    int ncellsY = matrix.size.y;
    int ncellsZ = matrix.size.z;

    pointsZ = ncellsZ + 1;			//initialize global variable (for extra speed) 
    YtimeZ = (ncellsY + 1)*pointsZ;

    f32v4 *verts[8];			//vertices of a cube (array of pointers for extra speed)
    f32v3 intVerts[12];			//linearly interpolated vertices on each edge
    int cubeIndex;					//shows which vertices are outside/inside
    int edgeIndex;					//index returned by edgeTable[cubeIndex]
    f32v4 gradVerts[8];			//gradients at each vertex of a cube		
    f32v3 grads[12];				//linearly interpolated gradients on each edge
    int indGrad;					//shows which gradients already have been computed
    int ind, ni, nj;				//ind: index of vertex 0
    //factor by which corresponding coordinates of gradient vectors are scaled
    f32v3 factor(1.0f / (2.0*gradFactorX), 1.0f / (2.0*gradFactorY), 1.0f / (2.0*gradFactorZ));

    f32v3 mainOffset(-(matrix.size.x / 2.0f), -(matrix.size.y / 2.0f), -(matrix.size.z / 2.0f));

    //MAIN LOOP: goes through all the points
    for (int i = 0; i < ncellsX; i++) {			//x axis
        ni = i*YtimeZ;
        for (int j = 0; j < ncellsY; j++) {		//y axis
            nj = j*pointsZ;
            for (int k = 0; k < ncellsZ; k++, ind++)	//z axis
            {
                //initialize vertices
                ind = ni + nj + k;
                verts[0] = &points[ind];
                verts[1] = &points[ind + YtimeZ];
                verts[4] = &points[ind + pointsZ];
                verts[5] = &points[ind + YtimeZ + pointsZ];
                verts[2] = &points[ind + YtimeZ + 1];
                verts[3] = &points[ind + 1];
                verts[6] = &points[ind + YtimeZ + pointsZ + 1];
                verts[7] = &points[ind + pointsZ + 1];

                //get the index
                cubeIndex = int(0);
                for (int n = 0; n < 8; n++)
                    if (verts[n]->w <= minValue) cubeIndex |= (1 << n);

                //check if its completely inside or outside
                if (!edgeTable[cubeIndex]) continue;

                indGrad = int(0);
                edgeIndex = edgeTable[cubeIndex];

                if (edgeIndex & 1) {
                    intVerts[0] = linearInterp(*verts[0], *verts[1], minValue);
                    if (i != 0 && j != 0 && k != 0) gradVerts[0] = CALC_GRAD_VERT_0(*verts)
                    else gradVerts[0] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    if (i != ncellsX - 1 && j != 0 && k != 0) gradVerts[1] = CALC_GRAD_VERT_1(*verts)
                    else gradVerts[1] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    indGrad |= 3;
                    grads[0] = linearInterp(gradVerts[0], gradVerts[1], minValue);
                    grads[0].x *= factor.x; grads[0].y *= factor.y; grads[0].z *= factor.z;
                }
                if (edgeIndex & 2) {
                    intVerts[1] = linearInterp(*verts[1], *verts[2], minValue);
                    if (!(indGrad & 2)) {
                        if (i != ncellsX - 1 && j != 0 && k != 0) gradVerts[1] = CALC_GRAD_VERT_1(*verts)
                        else gradVerts[1] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 2;
                    }
                    if (i != ncellsX - 1 && j != 0 && k != 0) gradVerts[2] = CALC_GRAD_VERT_2(*verts)
                    else gradVerts[2] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    indGrad |= 4;
                    grads[1] = linearInterp(gradVerts[1], gradVerts[2], minValue);
                    grads[1].x *= factor.x; grads[1].y *= factor.y; grads[1].z *= factor.z;
                }
                if (edgeIndex & 4) {
                    intVerts[2] = linearInterp(*verts[2], *verts[3], minValue);
                    if (!(indGrad & 4)) {
                        if (i != ncellsX - 1 && j != 0 && k != 0) gradVerts[2] = CALC_GRAD_VERT_2(*verts)
                        else gradVerts[2] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 4;
                    }
                    if (i != 0 && j != 0 && k != ncellsZ - 1) gradVerts[3] = CALC_GRAD_VERT_3(*verts)
                    else gradVerts[3] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    indGrad |= 8;
                    grads[2] = linearInterp(gradVerts[2], gradVerts[3], minValue);
                    grads[2].x *= factor.x; grads[2].y *= factor.y; grads[2].z *= factor.z;
                }
                if (edgeIndex & 8) {
                    intVerts[3] = linearInterp(*verts[3], *verts[0], minValue);
                    if (!(indGrad & 8)) {
                        if (i != 0 && j != 0 && k != ncellsZ - 1) gradVerts[3] = CALC_GRAD_VERT_3(*verts)
                        else gradVerts[3] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 8;
                    }
                    if (!(indGrad & 1)) {
                        if (i != 0 && j != 0 && k != 0) gradVerts[0] = CALC_GRAD_VERT_0(*verts)
                        else gradVerts[0] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 1;
                    }
                    grads[3] = linearInterp(gradVerts[3], gradVerts[0], minValue);
                    grads[3].x *= factor.x; grads[3].y *= factor.y; grads[3].z *= factor.z;
                }
                if (edgeIndex & 16) {
                    intVerts[4] = linearInterp(*verts[4], *verts[5], minValue);

                    if (i != 0 && j != ncellsY - 1 && k != 0) gradVerts[4] = CALC_GRAD_VERT_4(*verts)
                    else gradVerts[4] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);

                    if (i != ncellsX - 1 && j != ncellsY - 1 && k != 0) gradVerts[5] = CALC_GRAD_VERT_5(*verts)
                    else gradVerts[5] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);

                    indGrad |= 48;
                    grads[4] = linearInterp(gradVerts[4], gradVerts[5], minValue);
                    grads[4].x *= factor.x; grads[4].y *= factor.y; grads[4].z *= factor.z;
                }
                if (edgeIndex & 32) {
                    intVerts[5] = linearInterp(*verts[5], *verts[6], minValue);
                    if (!(indGrad & 32)) {
                        if (i != ncellsX - 1 && j != ncellsY - 1 && k != 0) gradVerts[5] = CALC_GRAD_VERT_5(*verts)
                        else gradVerts[5] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 32;
                    }

                    if (i != ncellsX - 1 && j != ncellsY - 1 && k != ncellsZ - 1) gradVerts[6] = CALC_GRAD_VERT_6(*verts)
                    else gradVerts[6] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    indGrad |= 64;
                    grads[5] = linearInterp(gradVerts[5], gradVerts[6], minValue);
                    grads[5].x *= factor.x; grads[5].y *= factor.y; grads[5].z *= factor.z;
                }
                if (edgeIndex & 64) {
                    intVerts[6] = linearInterp(*verts[6], *verts[7], minValue);
                    if (!(indGrad & 64)) {
                        if (i != ncellsX - 1 && j != ncellsY - 1 && k != ncellsZ - 1) gradVerts[6] = CALC_GRAD_VERT_6(*verts)
                        else gradVerts[6] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 64;
                    }

                    if (i != 0 && j != ncellsY - 1 && k != ncellsZ - 1) gradVerts[7] = CALC_GRAD_VERT_7(*verts)
                    else gradVerts[7] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    indGrad |= 128;
                    grads[6] = linearInterp(gradVerts[6], gradVerts[7], minValue);
                    grads[6].x *= factor.x; grads[6].y *= factor.y; grads[6].z *= factor.z;
                }
                if (edgeIndex & 128) {
                    intVerts[7] = linearInterp(*verts[7], *verts[4], minValue);
                    if (!(indGrad & 128)) {
                        if (i != 0 && j != ncellsY - 1 && k != ncellsZ - 1) gradVerts[7] = CALC_GRAD_VERT_7(*verts)
                        else gradVerts[7] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 128;
                    }
                    if (!(indGrad & 16)) {
                        if (i != 0 && j != ncellsY - 1 && k != 0) gradVerts[4] = CALC_GRAD_VERT_4(*verts)
                        else gradVerts[4] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 16;
                    }
                    grads[7] = linearInterp(gradVerts[7], gradVerts[4], minValue);
                    grads[7].x *= factor.x; grads[7].y *= factor.y; grads[7].z *= factor.z;
                }
                if (edgeIndex & 256) {
                    intVerts[8] = linearInterp(*verts[0], *verts[4], minValue);
                    if (!(indGrad & 1)) {
                        if (i != 0 && j != 0 && k != 0) gradVerts[0] = CALC_GRAD_VERT_0(*verts)
                        else gradVerts[0] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 1;
                    }
                    if (!(indGrad & 16)) {
                        if (i != 0 && j != ncellsY - 1 && k != 0) gradVerts[4] = CALC_GRAD_VERT_4(*verts)
                        else gradVerts[4] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 16;
                    }
                    grads[8] = linearInterp(gradVerts[0], gradVerts[4], minValue);
                    grads[8].x *= factor.x; grads[8].y *= factor.y; grads[8].z *= factor.z;
                }
                if (edgeIndex & 512) {
                    intVerts[9] = linearInterp(*verts[1], *verts[5], minValue);
                    if (!(indGrad & 2)) {
                        if (i != ncellsX - 1 && j != 0 && k != 0) gradVerts[1] = CALC_GRAD_VERT_1(*verts)
                        else gradVerts[1] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 2;
                    }
                    if (!(indGrad & 32)) {
                        if (i != ncellsX - 1 && j != ncellsY - 1 && k != 0) gradVerts[5] = CALC_GRAD_VERT_5(*verts)
                        else gradVerts[5] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 32;
                    }
                    grads[9] = linearInterp(gradVerts[1], gradVerts[5], minValue);
                    grads[9].x *= factor.x; grads[9].y *= factor.y; grads[9].z *= factor.z;
                }
                if (edgeIndex & 1024) {
                    intVerts[10] = linearInterp(*verts[2], *verts[6], minValue);
                    if (!(indGrad & 4)) {
                        if (i != ncellsX - 1 && j != 0 && k != 0) gradVerts[2] = CALC_GRAD_VERT_2(*verts)
                        else gradVerts[5] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 4;
                    }
                    if (!(indGrad & 64)) {
                        if (i != ncellsX - 1 && j != ncellsY - 1 && k != ncellsZ - 1) gradVerts[6] = CALC_GRAD_VERT_6(*verts)
                        else gradVerts[6] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 64;
                    }
                    grads[10] = linearInterp(gradVerts[2], gradVerts[6], minValue);
                    grads[10].x *= factor.x; grads[10].y *= factor.y; grads[10].z *= factor.z;
                }
                if (edgeIndex & 2048) {
                    intVerts[11] = linearInterp(*verts[3], *verts[7], minValue);
                    if (!(indGrad & 8)) {
                        if (i != 0 && j != 0 && k != ncellsZ - 1) gradVerts[3] = CALC_GRAD_VERT_3(*verts)
                        else gradVerts[3] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 8;
                    }
                    if (!(indGrad & 128)) {
                        if (i != 0 && j != ncellsY - 1 && k != ncellsZ - 1) gradVerts[7] = CALC_GRAD_VERT_7(*verts)
                        else gradVerts[7] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 128;
                    }
                    grads[11] = linearInterp(gradVerts[3], gradVerts[7], minValue);
                    grads[11].x *= factor.x; grads[11].y *= factor.y; grads[11].z *= factor.z;
                }

                //now build the triangles using triTable
                for (int n = 0; triTable[cubeIndex][n] != -1; n += 3) {
                    int index[3] = { triTable[cubeIndex][n + 2], triTable[cubeIndex][n + 1], triTable[cubeIndex][n] };
                    int startVertex = vertices.size();
                    vertices.resize(vertices.size() + 3);
                    for (int h = 0; h < 3; h++) {
                        vertices[startVertex + h].color = getColor(intVerts[index[h]], matrix);
                        vertices[startVertex + h].pos = intVerts[index[h]] + mainOffset;
                        vertices[startVertex + h].normal = grads[index[h]];
                    }
                }
            }
        }
    }
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