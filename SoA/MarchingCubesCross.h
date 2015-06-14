/////////////////////////////////////////////////////////////////////////////////////////////
//	FileName:	MarchingCubesCross.h
//	Author	:	Michael Y. Polyakov
//	email	:	myp@andrew.cmu.edu  or  mikepolyakov@hotmail.com
//	website	:	www.angelfire.com/linux/myp
//	date	:	July 2002
//	
//	Description:	Basics of Marching Cubes Algorithm
//					Computes normal vectors as cross product of edges of each triangle - hence 
//					the name ends in Cross.
//		(Warning: not efficient or right in terms of light information)
//		For more efficient source code and tutorial on Marching Cubes please visit www.angelfire.com/linux/myp
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MARCHINGCUBES_H_
#define MARCHINGCUBES_H_

#include "MarchingCubesTable.h"		//tables used by Marching Cubes (edgeTable and triTable)

//used to save triangles - 3 vertices and a normal vector
typedef struct {
    f32v3 p[3];
    f32v3 norm;
} TRIANGLE;

////////////////////////////////////////////////////////////////////////////////////////
//POINTERS TO FUNCTIONS																////
//pointer to function which computes the value at a point							////
typedef float(*FORMULA)(f32v3);													////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
///// the MARCHING CUBES algorithm /////											////
////////////////////////////////////////////////////////////////////////////////////////

//	Version 1
//Runs Marching Cubes on 3D array of points which already defined coordinates and values.
// ncellsX, ncellsY, ncellsZ):  number of cells to subdivide on each axis
// minValue: minimum to which value at every vertex is compared to
// points: array of length (ncellsX+1)(ncellsY+1)(ncellsZ+1) of f32v4 points containing coordinates and values
//returns pointer to triangle array and the number of triangles in numTriangles
//note: array of points is first taken on z axis, then y and then x. So for example, if u iterate through it in a
//       for loop, have indexes i, j, k for x, y, z respectively, then to get the point you will have to make the
//		 following index: i*(ncellsY+1)*(ncellsZ+1) + j*(ncellsZ+1) + k .
//		Also, the array starts at the minimum on all axes. (point at indices (i,j,k) is the minimum)
TRIANGLE* MarchingCubesCross(int ncellsX, int ncellsY, int ncellsZ,
                             float minValue, f32v4 * points, int &numTriangles);

//	Version 2
//First computes the 3D array of coordintes and values and then runs the above function on the data
//takes dimensions (minx,maxx,miny,...) and the number of cells (ncellsX,...) to subdivide on each axis
// minValue: minimum to which value at every vertex is compared to
// function of type float (f32v3 p) formula, which computes value of p at its coordinates
// function of type f32v3 (f32v4 p1, f32v4 p2) intersection, which determines the 
//  point of intersection of the surface and the edge between points p1 and p2
// saves number of triangles in numTriangles and the pointer to them is returned
// (note: mins' and maxs' are included in the algorithm)
TRIANGLE* MarchingCubesCross(float mcMinX, float mcMaxX, float mcMinY, float mcMaxY, float mcMinZ, float mcMaxZ,
                             int ncellsX, int ncellsY, int ncellsZ, float minValue,
                             FORMULA formula, int &numTriangles);
#endif