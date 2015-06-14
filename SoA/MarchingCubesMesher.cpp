/////////////////////////////////////////////////////////////////////////////////////////////
//	FileName:	MarchingCubesMesher.cpp
//	Author	:	Michael Y. Polyakov
//	email	:	myp@andrew.cmu.edu  or  mikepolyakov@hotmail.com
//	website	:	www.angelfire.com/linux/myp
//	date	:	July 2002
//	
//	Description:	'Straight' and Recursive Marching Cubes Algorithms
//					Recursive method is faster than the 'straight' one, especially when intersection does not 
//						have to be searched for every time.
//				Normal vectors are defined for each vertex as a gradients
//				For function definitions see MarchingCubesMesher.h
//				For a tutorial on Marching Cubes please visit www.angelfire.com/myp/linux
//
//	Please email me with any suggestion/bugs.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MarchingCubesMesher.h"
#include <math.h>

//Linear Interpolation between two points
f32v3 LinearInterp(const f32v4& p1, const f32v4& p2, float value) {
    f32v3 p13(p1);
    if (fabs(p1.w - p2.w) > 0.00001f)
        return p13 + (f32v3(p2) - p13) / (p2.w - p1.w)*(value - p1.w);
    else
        return p13;
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


//////////////////////////////////////////////////////////////////////////////////////////////////////
//	'STRAIGHT' MARCHING CUBES	ALGORITHM  ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//for gradients at the edges values of 1.0f, 1.0f, 1.0f, 1.0f  are given
TRIANGLE* MarchingCubes(int ncellsX, int ncellsY, int ncellsZ,
                        float gradFactorX, float gradFactorY, float gradFactorZ,
                        float minValue, f32v4 * points, int &numTriangles) {
    //this should be enough space, if not change 3 to 4
    TRIANGLE * triangles = new TRIANGLE[3 * ncellsX*ncellsY*ncellsZ];
    numTriangles = int(0);

    pointsZ = ncellsZ + 1;			//initialize global variable (for extra speed) 
    YtimeZ = (ncellsY + 1)*pointsZ;
    int lastX = ncellsX;			//left from older version
    int lastY = ncellsY;
    int lastZ = ncellsZ;

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

    //MAIN LOOP: goes through all the points
    for (int i = 0; i < lastX; i++) {			//x axis
        ni = i*YtimeZ;
        for (int j = 0; j < lastY; j++) {		//y axis
            nj = j*pointsZ;
            for (int k = 0; k < lastZ; k++, ind++)	//z axis
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
                    intVerts[0] = LinearInterp(*verts[0], *verts[1], minValue);
                    if (i != 0 && j != 0 && k != 0) gradVerts[0] = CALC_GRAD_VERT_0(*verts)
                    else gradVerts[0] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    if (i != lastX - 1 && j != 0 && k != 0) gradVerts[1] = CALC_GRAD_VERT_1(*verts)
                    else gradVerts[1] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    indGrad |= 3;
                    grads[0] = LinearInterp(gradVerts[0], gradVerts[1], minValue);
                    grads[0].x *= factor.x; grads[0].y *= factor.y; grads[0].z *= factor.z;
                }
                if (edgeIndex & 2) {
                    intVerts[1] = LinearInterp(*verts[1], *verts[2], minValue);
                    if (!(indGrad & 2)) {
                        if (i != lastX - 1 && j != 0 && k != 0) gradVerts[1] = CALC_GRAD_VERT_1(*verts)
                        else gradVerts[1] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 2;
                    }
                    if (i != lastX - 1 && j != 0 && k != 0) gradVerts[2] = CALC_GRAD_VERT_2(*verts)
                    else gradVerts[2] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    indGrad |= 4;
                    grads[1] = LinearInterp(gradVerts[1], gradVerts[2], minValue);
                    grads[1].x *= factor.x; grads[1].y *= factor.y; grads[1].z *= factor.z;
                }
                if (edgeIndex & 4) {
                    intVerts[2] = LinearInterp(*verts[2], *verts[3], minValue);
                    if (!(indGrad & 4)) {
                        if (i != lastX - 1 && j != 0 && k != 0) gradVerts[2] = CALC_GRAD_VERT_2(*verts)
                        else gradVerts[2] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 4;
                    }
                    if (i != 0 && j != 0 && k != lastZ - 1) gradVerts[3] = CALC_GRAD_VERT_3(*verts)
                    else gradVerts[3] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    indGrad |= 8;
                    grads[2] = LinearInterp(gradVerts[2], gradVerts[3], minValue);
                    grads[2].x *= factor.x; grads[2].y *= factor.y; grads[2].z *= factor.z;
                }
                if (edgeIndex & 8) {
                    intVerts[3] = LinearInterp(*verts[3], *verts[0], minValue);
                    if (!(indGrad & 8)) {
                        if (i != 0 && j != 0 && k != lastZ - 1) gradVerts[3] = CALC_GRAD_VERT_3(*verts)
                        else gradVerts[3] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 8;
                    }
                    if (!(indGrad & 1)) {
                        if (i != 0 && j != 0 && k != 0) gradVerts[0] = CALC_GRAD_VERT_0(*verts)
                        else gradVerts[0] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 1;
                    }
                    grads[3] = LinearInterp(gradVerts[3], gradVerts[0], minValue);
                    grads[3].x *= factor.x; grads[3].y *= factor.y; grads[3].z *= factor.z;
                }
                if (edgeIndex & 16) {
                    intVerts[4] = LinearInterp(*verts[4], *verts[5], minValue);

                    if (i != 0 && j != lastY - 1 && k != 0) gradVerts[4] = CALC_GRAD_VERT_4(*verts)
                    else gradVerts[4] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);

                    if (i != lastX - 1 && j != lastY - 1 && k != 0) gradVerts[5] = CALC_GRAD_VERT_5(*verts)
                    else gradVerts[5] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);

                    indGrad |= 48;
                    grads[4] = LinearInterp(gradVerts[4], gradVerts[5], minValue);
                    grads[4].x *= factor.x; grads[4].y *= factor.y; grads[4].z *= factor.z;
                }
                if (edgeIndex & 32) {
                    intVerts[5] = LinearInterp(*verts[5], *verts[6], minValue);
                    if (!(indGrad & 32)) {
                        if (i != lastX - 1 && j != lastY - 1 && k != 0) gradVerts[5] = CALC_GRAD_VERT_5(*verts)
                        else gradVerts[5] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 32;
                    }

                    if (i != lastX - 1 && j != lastY - 1 && k != lastZ - 1) gradVerts[6] = CALC_GRAD_VERT_6(*verts)
                    else gradVerts[6] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    indGrad |= 64;
                    grads[5] = LinearInterp(gradVerts[5], gradVerts[6], minValue);
                    grads[5].x *= factor.x; grads[5].y *= factor.y; grads[5].z *= factor.z;
                }
                if (edgeIndex & 64) {
                    intVerts[6] = LinearInterp(*verts[6], *verts[7], minValue);
                    if (!(indGrad & 64)) {
                        if (i != lastX - 1 && j != lastY - 1 && k != lastZ - 1) gradVerts[6] = CALC_GRAD_VERT_6(*verts)
                        else gradVerts[6] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 64;
                    }

                    if (i != 0 && j != lastY - 1 && k != lastZ - 1) gradVerts[7] = CALC_GRAD_VERT_7(*verts)
                    else gradVerts[7] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                    indGrad |= 128;
                    grads[6] = LinearInterp(gradVerts[6], gradVerts[7], minValue);
                    grads[6].x *= factor.x; grads[6].y *= factor.y; grads[6].z *= factor.z;
                }
                if (edgeIndex & 128) {
                    intVerts[7] = LinearInterp(*verts[7], *verts[4], minValue);
                    if (!(indGrad & 128)) {
                        if (i != 0 && j != lastY - 1 && k != lastZ - 1) gradVerts[7] = CALC_GRAD_VERT_7(*verts)
                        else gradVerts[7] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 128;
                    }
                    if (!(indGrad & 16)) {
                        if (i != 0 && j != lastY - 1 && k != 0) gradVerts[4] = CALC_GRAD_VERT_4(*verts)
                        else gradVerts[4] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 16;
                    }
                    grads[7] = LinearInterp(gradVerts[7], gradVerts[4], minValue);
                    grads[7].x *= factor.x; grads[7].y *= factor.y; grads[7].z *= factor.z;
                }
                if (edgeIndex & 256) {
                    intVerts[8] = LinearInterp(*verts[0], *verts[4], minValue);
                    if (!(indGrad & 1)) {
                        if (i != 0 && j != 0 && k != 0) gradVerts[0] = CALC_GRAD_VERT_0(*verts)
                        else gradVerts[0] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 1;
                    }
                    if (!(indGrad & 16)) {
                        if (i != 0 && j != lastY - 1 && k != 0) gradVerts[4] = CALC_GRAD_VERT_4(*verts)
                        else gradVerts[4] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 16;
                    }
                    grads[8] = LinearInterp(gradVerts[0], gradVerts[4], minValue);
                    grads[8].x *= factor.x; grads[8].y *= factor.y; grads[8].z *= factor.z;
                }
                if (edgeIndex & 512) {
                    intVerts[9] = LinearInterp(*verts[1], *verts[5], minValue);
                    if (!(indGrad & 2)) {
                        if (i != lastX - 1 && j != 0 && k != 0) gradVerts[1] = CALC_GRAD_VERT_1(*verts)
                        else gradVerts[1] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 2;
                    }
                    if (!(indGrad & 32)) {
                        if (i != lastX - 1 && j != lastY - 1 && k != 0) gradVerts[5] = CALC_GRAD_VERT_5(*verts)
                        else gradVerts[5] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 32;
                    }
                    grads[9] = LinearInterp(gradVerts[1], gradVerts[5], minValue);
                    grads[9].x *= factor.x; grads[9].y *= factor.y; grads[9].z *= factor.z;
                }
                if (edgeIndex & 1024) {
                    intVerts[10] = LinearInterp(*verts[2], *verts[6], minValue);
                    if (!(indGrad & 4)) {
                        if (i != lastX - 1 && j != 0 && k != 0) gradVerts[2] = CALC_GRAD_VERT_2(*verts)
                        else gradVerts[5] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 4;
                    }
                    if (!(indGrad & 64)) {
                        if (i != lastX - 1 && j != lastY - 1 && k != lastZ - 1) gradVerts[6] = CALC_GRAD_VERT_6(*verts)
                        else gradVerts[6] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 64;
                    }
                    grads[10] = LinearInterp(gradVerts[2], gradVerts[6], minValue);
                    grads[10].x *= factor.x; grads[10].y *= factor.y; grads[10].z *= factor.z;
                }
                if (edgeIndex & 2048) {
                    intVerts[11] = LinearInterp(*verts[3], *verts[7], minValue);
                    if (!(indGrad & 8)) {
                        if (i != 0 && j != 0 && k != lastZ - 1) gradVerts[3] = CALC_GRAD_VERT_3(*verts)
                        else gradVerts[3] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 8;
                    }
                    if (!(indGrad & 128)) {
                        if (i != 0 && j != lastY - 1 && k != lastZ - 1) gradVerts[7] = CALC_GRAD_VERT_7(*verts)
                        else gradVerts[7] = f32v4(1.0f, 1.0f, 1.0f, 1.0f);
                        indGrad |= 128;
                    }
                    grads[11] = LinearInterp(gradVerts[3], gradVerts[7], minValue);
                    grads[11].x *= factor.x; grads[11].y *= factor.y; grads[11].z *= factor.z;
                }

                //DebugWrite("2");

                //now build the triangles using triTable
                for (int n = 0; triTable[cubeIndex][n] != -1; n += 3) {
                    int index[3] = { triTable[cubeIndex][n + 2], triTable[cubeIndex][n + 1], triTable[cubeIndex][n] };
                    for (int h = 0; h < 3; h++) {	//copy vertices and normals into triangles array
                        triangles[numTriangles].p[h] = intVerts[index[h]];
                        triangles[numTriangles].norm[h] = grads[index[h]];
                    }
                    numTriangles++;	//one more triangle has been added
                }
            }
        }
    }

    //free all wasted space
    TRIANGLE * retTriangles = new TRIANGLE[numTriangles];
    for (int i = 0; i < numTriangles; i++)
        retTriangles[i] = triangles[i];
    delete[] triangles;

    return retTriangles;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	RECURSIVE MARCHING CUBES ALGORITHM ////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  MACROS  ///////////////////////////////////////////////////////////////////////////////////////////////////
//these macros initialize data and then run marching cubes on the cube with the surface having the specified 
//	number as 'recieving' data (but number of that surface for the current cube is going to be 'opposite').
//	Each runs the corresponding recursive function
//	For numbering, to see which indices of prevVerts,... correspong to indices of the current cube, see
//	my webpage at www.angelfire.com/linux/myp

#define MC_FACE0																							\
{																											\
  if(! marchedCubes[ind - 1]) {																				\
	passGradIndex = 0;																						\
	if(gradIndex & 1) passGradIndex |= 8;																	\
	if(gradIndex & 2) passGradIndex |= 4;																	\
	if(gradIndex & 32) passGradIndex |= 64;																	\
	if(gradIndex & 16) passGradIndex |= 128;																\
	passEdgeIndex = 0;																						\
	if(edgeIndex & 1) passEdgeIndex |= 4;																	\
	if(edgeIndex & 512) passGradIndex |= 1024;																\
	if(edgeIndex & 16) passEdgeIndex |= 64;																	\
	if(edgeIndex & 256) passGradIndex |= 2048;																\
	prevVerts[0] = verts[0]; prevVerts[1] = verts[1]; prevVerts[2] = verts[5]; prevVerts[3] = verts[4];		\
	prevIntVerts[0] = intVerts[0]; prevIntVerts[1] = intVerts[9];											\
	prevIntVerts[2] = intVerts[4]; prevIntVerts[3] = intVerts[8];											\
	prevGradVerts[0] = gradVerts[0]; prevGradVerts[1] = gradVerts[1];										\
	prevGradVerts[2] = gradVerts[5]; prevGradVerts[3] = gradVerts[4];										\
	prevGrads[0] = grads[0]; prevGrads[1] = grads[9]; prevGrads[2] = grads[4]; prevGrads[3] = grads[8];		\
	triangles = MCFace0(ncellsX, ncellsY, ncellsZ, gradFactorX, gradFactorY, gradFactorZ,					\
							ind-1, i, j, k-1, minValue, points, triangles, numTriangles,					\
							prevVerts, prevIntVerts, passEdgeIndex,											\
							prevGradVerts, prevGrads, passGradIndex, marchedCubes);							\
      }																											\
}

#define MC_FACE1																							\
{																											\
  if(! marchedCubes[ind + YtimeZ]) {																		\
	passGradIndex = 0;																						\
	if(gradIndex & 4) passGradIndex |= 8;																	\
	if(gradIndex & 2) passGradIndex |= 1;																	\
	if(gradIndex & 32) passGradIndex |= 16;																	\
	if(gradIndex & 64) passGradIndex |= 128;																\
	passEdgeIndex = 0;																						\
	if(edgeIndex & 2) passEdgeIndex |= 8;																	\
	if(edgeIndex & 512) passEdgeIndex |= 256;																\
	if(edgeIndex & 32) passEdgeIndex |= 128;																\
	if(edgeIndex & 1024) passEdgeIndex |= 2048;																\
	prevVerts[0] = verts[2]; prevVerts[1] = verts[1]; prevVerts[2] = verts[5]; prevVerts[3] = verts[6];		\
	prevIntVerts[0] = intVerts[1]; prevIntVerts[1] = intVerts[9];											\
	prevIntVerts[2] = intVerts[5]; prevIntVerts[3] = intVerts[10];											\
	prevGradVerts[0] = gradVerts[2]; prevGradVerts[1] = gradVerts[1];										\
	prevGradVerts[2] = gradVerts[5]; prevGradVerts[3] = gradVerts[6];										\
	prevGrads[0] = grads[1]; prevGrads[1] = grads[9]; prevGrads[2] = grads[5]; prevGrads[3] = grads[10];	\
	triangles = MCFace1(ncellsX, ncellsY, ncellsZ, gradFactorX, gradFactorY, gradFactorZ,					\
							ind+YtimeZ, i+1, j, k, minValue, points, triangles, numTriangles,				\
							prevVerts, prevIntVerts, passEdgeIndex,											\
							prevGradVerts, prevGrads, passGradIndex,  marchedCubes);						\
      }																											\
}

#define MC_FACE2																							\
{																											\
  if(! marchedCubes[ind + 1]) {																				\
	passGradIndex = 0;																						\
	if(gradIndex & 8) passGradIndex |= 1;																	\
	if(gradIndex & 4) passGradIndex |= 2;																	\
	if(gradIndex & 64) passGradIndex |= 32;																	\
	if(gradIndex & 128) passGradIndex |= 16;																\
	passEdgeIndex = 0;																						\
	if(edgeIndex & 4) passEdgeIndex |= 1;																	\
	if(edgeIndex & 1024) passEdgeIndex |= 512;																\
	if(edgeIndex & 64) passEdgeIndex |= 16;																	\
	if(edgeIndex & 2048) passEdgeIndex |= 256;																\
	prevVerts[0] = verts[3]; prevVerts[1] = verts[2]; prevVerts[2] = verts[6]; prevVerts[3] = verts[7];		\
	prevIntVerts[0] = intVerts[2]; prevIntVerts[1] = intVerts[10];											\
	prevIntVerts[2] = intVerts[6]; prevIntVerts[3] = intVerts[11];											\
	prevGradVerts[0] = gradVerts[3]; prevGradVerts[1] = gradVerts[2];										\
	prevGradVerts[2] = gradVerts[6]; prevGradVerts[3] = gradVerts[7];										\
	prevGrads[0] = grads[2]; prevGrads[1] = grads[10]; prevGrads[2] = grads[6]; prevGrads[3] = grads[11];	\
	triangles = MCFace2(ncellsX, ncellsY, ncellsZ, gradFactorX, gradFactorY, gradFactorZ,					\
							ind+1, i, j, k+1, minValue, points, triangles, numTriangles,					\
							prevVerts, prevIntVerts, passEdgeIndex,											\
							prevGradVerts, prevGrads, passGradIndex, marchedCubes);							\
      }																											\
}

#define MC_FACE3																							\
{																											\
  if(! marchedCubes[ind - YtimeZ]) {																		\
	passGradIndex = 0;																						\
	if(gradIndex & 8) passGradIndex |= 4;																	\
	if(gradIndex & 1) passGradIndex |= 2;																	\
	if(gradIndex & 128) passGradIndex |= 64;																\
	if(gradIndex & 16) passGradIndex |= 32;																	\
	passEdgeIndex = 0;																						\
	if(edgeIndex & 8) passEdgeIndex |= 2;																	\
	if(edgeIndex & 256) passEdgeIndex |= 512;																\
	if(edgeIndex & 128) passEdgeIndex |= 32;																\
	if(edgeIndex & 2048) passEdgeIndex |= 1024;																\
	prevVerts[0] = verts[3]; prevVerts[1] = verts[0]; prevVerts[2] = verts[4]; prevVerts[3] = verts[7];		\
	prevIntVerts[0] = intVerts[3]; prevIntVerts[1] = intVerts[8];											\
	prevIntVerts[2] = intVerts[7]; prevIntVerts[3] = intVerts[11];											\
	prevGradVerts[0] = gradVerts[3]; prevGradVerts[1] = gradVerts[0];										\
	prevGradVerts[2] = gradVerts[4]; prevGradVerts[3] = gradVerts[7];										\
	prevGrads[0] = grads[3]; prevGrads[1] = grads[8]; prevGrads[2] = grads[7]; prevGrads[3] = grads[11];	\
	triangles = MCFace3(ncellsX, ncellsY, ncellsZ, gradFactorX, gradFactorY, gradFactorZ,					\
							ind-YtimeZ, i-1, j, k, minValue, points, triangles, numTriangles,				\
							prevVerts, prevIntVerts, passEdgeIndex,											\
							prevGradVerts, prevGrads, passGradIndex, marchedCubes);							\
      }																											\
}

//done
#define MC_FACE4																							\
{																											\
  if(! marchedCubes[ind + pointsZ]) {																		\
	passGradIndex = 0;																						\
	if(gradIndex & 128) passGradIndex |= 8;																	\
	if(gradIndex & 64) passGradIndex |= 4;																	\
	if(gradIndex & 32) passGradIndex |= 2;																	\
	if(gradIndex & 16) passGradIndex |= 1;																	\
	passEdgeIndex = 0;																						\
	if(edgeIndex & 128) passEdgeIndex |= 8;																	\
	if(edgeIndex & 64) passEdgeIndex |= 4;																	\
	if(edgeIndex & 32) passEdgeIndex |= 2;																	\
	if(edgeIndex & 16) passEdgeIndex |= 1;																	\
	prevVerts[0] = verts[7]; prevVerts[1] = verts[6]; prevVerts[2] = verts[5]; prevVerts[3] = verts[4];		\
	prevIntVerts[0] = intVerts[6]; prevIntVerts[1] = intVerts[5];											\
	prevIntVerts[2] = intVerts[4]; prevIntVerts[3] = intVerts[7];											\
	prevGradVerts[0] = gradVerts[7]; prevGradVerts[1] = gradVerts[6];										\
	prevGradVerts[2] = gradVerts[5]; prevGradVerts[3] = gradVerts[4];										\
	prevGrads[0] = grads[6]; prevGrads[1] = grads[5]; prevGrads[2] = grads[4]; prevGrads[3] = grads[7];		\
	triangles = MCFace4(ncellsX, ncellsY, ncellsZ, gradFactorX, gradFactorY, gradFactorZ,					\
							ind+pointsZ, i, j+1, k, minValue, points, triangles, numTriangles,				\
							prevVerts, prevIntVerts, passEdgeIndex,											\
							prevGradVerts, prevGrads, passGradIndex, marchedCubes);							\
      }																											\
}

#define MC_FACE5																							\
{																											\
  if(! marchedCubes[ind - ncellsZ - 1]) {																	\
	passGradIndex = (gradIndex << 4) & 240;																	\
	passEdgeIndex = (edgeIndex << 4) & 240;																	\
	prevVerts[0] = verts[3]; prevVerts[1] = verts[2]; prevVerts[2] = verts[1]; prevVerts[3] = verts[0];		\
	prevIntVerts[0] = intVerts[2]; prevIntVerts[1] = intVerts[1];											\
	prevIntVerts[2] = intVerts[0]; prevIntVerts[3] = intVerts[3];											\
	prevGradVerts[0] = gradVerts[3]; prevGradVerts[1] = gradVerts[2];										\
	prevGradVerts[2] = gradVerts[1]; prevGradVerts[3] = gradVerts[0];										\
	prevGrads[0] = grads[2]; prevGrads[1] = grads[1]; prevGrads[2] = grads[0]; prevGrads[3] = grads[3];		\
	triangles = MCFace5(ncellsX, ncellsY, ncellsZ, gradFactorX, gradFactorY, gradFactorZ,					\
							ind-ncellsZ-1, i, j-1, k, minValue, points, triangles, numTriangles,			\
							prevVerts, prevIntVerts, passEdgeIndex,											\
							prevGradVerts, prevGrads, passGradIndex, marchedCubes);							\
      }																											\
}
/// END FACE MACROS /////////////////////////////////////////////////////////////////////////////////////////
