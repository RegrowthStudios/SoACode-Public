/////////////////////////////////////////////////////////////////////////////////////////////
//	FileName:	MarchingCubes.cpp
//	Author	:	Michael Y. Polyakov
//	email	:	myp@andrew.cmu.edu  or  mikepolyakov@hotmail.com
//	website	:	www.angelfire.com/linux/myp
//	date	:	July 2002
//	
//	Description:	'Straight' and Recursive Marching Cubes Algorithms
//				Normal vectors are defined for each vertex as a gradients
//				For function definitions see MarchingCubes.h
//				For a tutorial on Marching Cubes please visit www.angelfire.com/myp/linux
//
//	Please email me with any suggestion/bugs.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MARCHINGCUBES_H_
#define MARCHINGCUBES_H_

#include "MarchingCubesTable.h"

//struct for storing triangle information - 3 vertices and 3 normal vectors for each vertex
typedef struct {
    f32v3 p[3];
    f32v3 norm[3];
} TRIANGLE;

/////////////////////////////////////////////////////////////////////////////////////////
///// MARCHING CUBES ALGORITHM /////
/////////////////////////////////////////////////////////////////////////////////////////

// 'STRAIGHT' Marching Cubes Algorithm //////////////////////////////////////////////////
//takes number of cells (ncellsX, ncellsY, ncellsZ) to subdivide on each axis
// minValue used to pass into LinearInterp
// gradFactor for each axis (multiplies each component of gradient vector by 1/(2*gradFactor) ).
//		Should be set to the length of a side (or close to it)
// array of length (ncellsX+1)(ncellsY+1)(ncellsZ+1) of f32v4 points containing coordinates and values
//returns pointer to triangle array and the number of triangles in numTriangles
//note: array of points is first taken on z axis, then y and then x. So for example, if you iterate through it in a
//       for loop, have indexes i, j, k for x, y, z respectively, then to get the point you will have to make the
//		 following index: i*(ncellsY+1)*(ncellsZ+1) + j*(ncellsZ+1) + k .
//		Also, the array starts at the minimum on all axes.
extern TRIANGLE* MarchingCubes(int ncellsX, int ncellsY, int ncellsZ, float gradFactorX, float gradFactorY, float gradFactorZ,
                        float minValue, f32v4 * points, int &numTriangles);
/////////////////////////////////////////////////////////////////////////////////////////

#endif