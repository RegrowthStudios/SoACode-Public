#include "stdafx.h"

#include "ChunkMesh.h"

#include "Chunk.h"

ChunkMesh::ChunkMesh(Chunk *ch) : vboID(0),
    vaoID(0),
    transVaoID(0),
    transVboID(0),
    cutoutVaoID(0), 
    cutoutVboID(0), 
    waterVboID(0), 
    vecIndex(-1),
    distance(30.0f),
    needsSort(true), 
    inFrustum(false), 
    position(ch->position)
{}
