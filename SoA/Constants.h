#pragma once
#define SOA_VERSION 0.1.5

#ifdef _DEBUG 
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif

#ifdef _DEBUG
//#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
//#define new DEBUG_NEW
#endif

const i32 CHUNK_WIDTH = 32;
const i32 CHUNK_LAYER = CHUNK_WIDTH*CHUNK_WIDTH;
const i32 CHUNK_SIZE = CHUNK_LAYER*CHUNK_WIDTH;
const i32 SURFACE_DEPTH = 256;
const i32 OBJECT_LIST_SIZE = 24096;

#ifndef M_PI
#define M_PI    3.14159265358979323846264338327950288   /* pi */
#endif
#define M_G 0.0000000000667384