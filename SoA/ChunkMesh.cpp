#include "stdafx.h"
#include "ChunkMesh.h"

#include "BlockData.h"
#include "NChunk.h"
#include "RenderTask.h"

KEG_ENUM_DEF(MeshType, MeshType, e) {
    e.addValue("none", MeshType::NONE);
    e.addValue("cube", MeshType::BLOCK);
    e.addValue("leaves", MeshType::LEAVES);
    e.addValue("triangle", MeshType::FLORA);
    e.addValue("cross", MeshType::CROSSFLORA);
    e.addValue("liquid", MeshType::LIQUID);
    e.addValue("flat", MeshType::FLAT);
}

void MesherInfo::init(const BlockPack* blocks, int dataWidth, int dataLayer) {
    this->blocks = blocks;
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

ChunkMeshData::ChunkMeshData() : type(RenderTaskType::DEFAULT) {
    // Empty
}

ChunkMeshData::ChunkMeshData(RenderTask *task) : chunk(task->chunk), type(task->type) {
    // Empty
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
