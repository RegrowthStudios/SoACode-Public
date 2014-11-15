#include "stdafx.h"

#include "ChunkMesh.h"

#include "Chunk.h"

#include "RenderTask.h"

void MesherInfo::init(int dataWidth, int dataLayer) {

    #define NUM_FACES 6
    // Set up the texture params
    pyBaseMethodParams.init(this, 1, -dataWidth, dataLayer, offsetof(BlockTextureFaces, BlockTextureFaces::py) / sizeof(ui32));
    pyOverlayMethodParams.init(this, 1, -dataWidth, dataLayer, offsetof(BlockTextureFaces, BlockTextureFaces::py) / sizeof(ui32) + NUM_FACES);

    nyBaseMethodParams.init(this, -1, -dataWidth, -dataLayer, offsetof(BlockTextureFaces, BlockTextureFaces::ny) / sizeof(ui32));
    nyOverlayMethodParams.init(this, -1, -dataWidth, -dataLayer, offsetof(BlockTextureFaces, BlockTextureFaces::ny) / sizeof(ui32) + NUM_FACES);

    pxBaseMethodParams.init(this, -dataWidth, dataLayer, 1, offsetof(BlockTextureFaces, BlockTextureFaces::px) / sizeof(ui32));
    pxOverlayMethodParams.init(this, -dataWidth, dataLayer, 1, offsetof(BlockTextureFaces, BlockTextureFaces::px) / sizeof(ui32) + NUM_FACES);

    nxBaseMethodParams.init(this, dataWidth, dataLayer, -1, offsetof(BlockTextureFaces, BlockTextureFaces::nx) / sizeof(ui32));
    nxOverlayMethodParams.init(this, dataWidth, dataLayer, -1, offsetof(BlockTextureFaces, BlockTextureFaces::nx) / sizeof(ui32) + NUM_FACES);

    pzBaseMethodParams.init(this, 1, dataLayer, dataWidth, offsetof(BlockTextureFaces, BlockTextureFaces::pz) / sizeof(ui32));
    pzOverlayMethodParams.init(this, 1, dataLayer, dataWidth, offsetof(BlockTextureFaces, BlockTextureFaces::pz) / sizeof(ui32) + NUM_FACES);

    nzBaseMethodParams.init(this, -1, dataLayer, -dataWidth, offsetof(BlockTextureFaces, BlockTextureFaces::nz) / sizeof(ui32));
    nzOverlayMethodParams.init(this, -1, dataLayer, -dataWidth, offsetof(BlockTextureFaces, BlockTextureFaces::nz) / sizeof(ui32) + NUM_FACES);
}

ChunkMesh::ChunkMesh(Chunk *ch) : vboID(0),
    vaoID(0),
    transVaoID(0),
    transVboID(0),
    transIndexID(0),
    cutoutVaoID(0), 
    cutoutVboID(0), 
    waterVboID(0), 
    vecIndex(UNINITIALIZED_INDEX),
    distance(30.0f),
    needsSort(true), 
    inFrustum(false), 
    position(ch->gridPosition)
{}

ChunkMeshData::ChunkMeshData(Chunk *ch) : chunk(ch), type(RenderTaskType::DEFAULT){
}

ChunkMeshData::ChunkMeshData(RenderTask *task) : chunk(task->chunk), type(task->type) {
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