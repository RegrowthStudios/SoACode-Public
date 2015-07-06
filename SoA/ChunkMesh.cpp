#include "stdafx.h"
#include "ChunkMesh.h"

#include "BlockData.h"
#include "Chunk.h"
#include "ChunkMeshTask.h"

KEG_ENUM_DEF(MeshType, MeshType, e) {
    e.addValue("none", MeshType::NONE);
    e.addValue("cube", MeshType::BLOCK);
    e.addValue("leaves", MeshType::LEAVES);
    e.addValue("triangle", MeshType::FLORA);
    e.addValue("cross", MeshType::CROSSFLORA);
    e.addValue("liquid", MeshType::LIQUID);
    e.addValue("flat", MeshType::FLAT);
}

ChunkMeshData::ChunkMeshData() : type(MeshTaskType::DEFAULT) {
    // Empty
}

ChunkMeshData::ChunkMeshData(MeshTaskType type) : type(type) {
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
