/////////////////////////////////////////////////////////////////////////////////////////////
//	FileName:	MarchingCubesCross.cpp
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
#include "stdafx.h"
#include "MarchingCubesCross.h"

//Linear Interpolation function
f32v3 LinearInterp(const f32v4& p1, const f32v4& p2, float value) {
    f32v3 p13 = f32v3(p1);
    if (p1.w != p2.w)
        return p13 + (f32v3(p2) - p13) / (p2.w - p1.w)*(value - p1.w);
    else
        return f32v3(p1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//	MARCHING CUBES	//
//////////////////////////////////////////////////////////////////////////////////////////////////////

//  VERSION  1  //
TRIANGLE* MarchingCubesCross(int ncellsX, int ncellsY, int ncellsZ,
                             float minValue, f32v4 * points, int &numTriangles) {
    //this should be enough space, if not change 3 to 4
    TRIANGLE * triangles = new TRIANGLE[3 * ncellsX*ncellsY*ncellsZ];
    numTriangles = int(0);

    int YtimeZ = (ncellsY + 1)*(ncellsZ + 1);	//for little extra speed
    int ni, nj;

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
                /*(step 3)*/ verts[0] = points[ind];
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
                    /*(step 4)*/		if (verts[n].w <= minValue) cubeIndex |= (1 << n);

                //check if its completely inside or outside
                /*(step 5)*/ if (!edgeTable[cubeIndex]) continue;

                //get linearly interpolated vertices on edges and save into the array
                f32v3 intVerts[12];
                /*(step 6)*/ if (edgeTable[cubeIndex] & 1) intVerts[0] = LinearInterp(verts[0], verts[1], minValue);
                if (edgeTable[cubeIndex] & 2) intVerts[1] = LinearInterp(verts[1], verts[2], minValue);
                if (edgeTable[cubeIndex] & 4) intVerts[2] = LinearInterp(verts[2], verts[3], minValue);
                if (edgeTable[cubeIndex] & 8) intVerts[3] = LinearInterp(verts[3], verts[0], minValue);
                if (edgeTable[cubeIndex] & 16) intVerts[4] = LinearInterp(verts[4], verts[5], minValue);
                if (edgeTable[cubeIndex] & 32) intVerts[5] = LinearInterp(verts[5], verts[6], minValue);
                if (edgeTable[cubeIndex] & 64) intVerts[6] = LinearInterp(verts[6], verts[7], minValue);
                if (edgeTable[cubeIndex] & 128) intVerts[7] = LinearInterp(verts[7], verts[4], minValue);
                if (edgeTable[cubeIndex] & 256) intVerts[8] = LinearInterp(verts[0], verts[4], minValue);
                if (edgeTable[cubeIndex] & 512) intVerts[9] = LinearInterp(verts[1], verts[5], minValue);
                if (edgeTable[cubeIndex] & 1024) intVerts[10] = LinearInterp(verts[2], verts[6], minValue);
                if (edgeTable[cubeIndex] & 2048) intVerts[11] = LinearInterp(verts[3], verts[7], minValue);

                //now build the triangles using triTable
                for (int n = 0; triTable[cubeIndex][n] != -1; n += 3) {
                    /*(step 7)*/ 	triangles[numTriangles].p[0] = intVerts[triTable[cubeIndex][n + 2]];
                    triangles[numTriangles].p[1] = intVerts[triTable[cubeIndex][n + 1]];
                    triangles[numTriangles].p[2] = intVerts[triTable[cubeIndex][n]];

                    f32v3 p1 = (triangles[numTriangles].p[1] - triangles[numTriangles].p[0]);
                    f32v3 p2 = (triangles[numTriangles].p[2] - triangles[numTriangles].p[0]);
                    //Computing normal as cross product of triangle's edges
                    /*(step 8)*/ 
                    triangles[numTriangles].norm = glm::normalize(glm::cross(p1, p2));
                    numTriangles++;
                }

            }	//END OF Z FOR LOOP
        }	//END OF Y FOR LOOP
    }	//END OF X FOR LOOP

    //free all the wasted space
    TRIANGLE * retTriangles = new TRIANGLE[numTriangles];
    for (int i = 0; i < numTriangles; i++)
        retTriangles[i] = triangles[i];
    delete[] triangles;

    return retTriangles;
}


//	VERSION  2  //
TRIANGLE* MarchingCubesCross(float mcMinX, float mcMaxX, float mcMinY, float mcMaxY, float mcMinZ, float mcMaxZ,
                             int ncellsX, int ncellsY, int ncellsZ, float minValue,
                             FORMULA formula, int &numTriangles) {
    //space is already defined and subdivided (mcMinX,...), staring with step 3
    //first initialize the points
    f32v4 * mcDataPoints = new f32v4[(ncellsX + 1)*(ncellsY + 1)*(ncellsZ + 1)];
    f32v3 stepSize((mcMaxX - mcMinX) / ncellsX, (mcMaxY - mcMinY) / ncellsY, (mcMaxZ - mcMinZ) / ncellsZ);

    int YtimesZ = (ncellsY + 1)*(ncellsZ + 1);		//for little extra speed
    for (int i = 0; i < ncellsX + 1; i++) {
        int ni = i*YtimesZ;						//for little extra speed
        float vertX = mcMinX + i*stepSize.x;
        for (int j = 0; j < ncellsY + 1; j++) {
            int nj = j*(ncellsZ + 1);				//for little extra speed
            float vertY = mcMinY + j*stepSize.y;
            for (int k = 0; k < ncellsZ + 1; k++) {
                f32v4 vert(vertX, vertY, mcMinZ + k*stepSize.z, 0);
                vert.w = formula((f32v3)vert);
                /*(step 3)*/ mcDataPoints[ni + nj + k] = vert;
            }
        }
    }
    //then run Marching Cubes (version 1) on the data
    return MarchingCubesCross(ncellsX, ncellsY, ncellsZ, minValue, mcDataPoints, numTriangles);
}