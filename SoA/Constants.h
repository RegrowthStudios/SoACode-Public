///
/// Constants.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 17 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Defines helpful SoA specific constants
///

#pragma once

#ifndef Constants_h__
#define Constants_h__

#define SOA_VERSION 0.1.5

#ifdef _DEBUG 
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif

// Uncomment for advanced heap checking
#ifdef _DEBUG
//#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__) 
//#define new DEBUG_NEW
#endif

const i32 CHUNK_WIDTH = 32;
// Chunk Width Minus 1
const i32 CHUNK_WIDTH_M1 = 31;
const i32 HALF_CHUNK_WIDTH = CHUNK_WIDTH / 2;
const i32 CHUNK_LAYER = CHUNK_WIDTH*CHUNK_WIDTH;
const i32 CHUNK_SIZE = CHUNK_LAYER*CHUNK_WIDTH;
const i32 SURFACE_DEPTH = 256;
const i32 OBJECT_LIST_SIZE = 24096;

#ifndef M_PI
#define M_PI   3.14159265358979323846264338327950288   /* pi */
#endif
#ifndef M_2_PI
#define M_2_PI 6.283185307179586476925286766559 /* 2 * pi */
#endif
#ifndef M_4_PI
#define M_4_PI 12.566370614359172953850573533118 /* 4 * pi */
#endif
#ifndef M_PI_2
#define M_PI_2 1.5707963267948966192313216916398   /* pi / 2 */
#endif
#define M_G 0.0000000000667384
#define M_SOL 1989100000000000000000000000000.0

/*** Helpful conversion factors ***/
const f64 KM_PER_M = 0.001;
const f64 M_PER_KM = 1000.0;
const f64 KM_PER_VOXEL = 0.0005;
const f64 M_PER_VOXEL = 0.5;
const f64 VOXELS_PER_M = 2.0;
const f64 VOXELS_PER_KM = 2000.0;

// Useful sentinal value to use in place of DBL_MAX for distance checks and such,
// since DBL_MAX is just too damn big and can overflow with any math
const f64 DOUBLE_SENTINEL = 10000000000000000000000000000000000000000000.0;

const f64 DEG_TO_RAD = M_PI / 180.0;

#endif // Constants_h__
