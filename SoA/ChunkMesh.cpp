#include "stdafx.h"

#include "ChunkMesh.h"

#include "Chunk.h"

#include "RenderTask.h"

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

ChunkMeshData::ChunkMeshData(Chunk *ch) : chunk(ch), transVertIndex(0), type(MeshJobType::DEFAULT) {
}

ChunkMeshData::ChunkMeshData(RenderTask *task) : chunk(task->chunk), transVertIndex(0), type(task->type) {
}

void ChunkMeshData::addTransQuad(const i8v3& pos) {
    transQuadPositions.push_back(pos);

    int size = transQuadIndices.size();
    transQuadIndices.resize(size + 6);
    transQuadIndices[size++] = transVertIndex;
    transQuadIndices[size++] = transVertIndex + 1;
    transQuadIndices[size++] = transVertIndex + 2;
    transQuadIndices[size++] = transVertIndex + 2;
    transQuadIndices[size++] = transVertIndex + 3;
    transQuadIndices[size] = transVertIndex;

    transVertIndex += 4;
}