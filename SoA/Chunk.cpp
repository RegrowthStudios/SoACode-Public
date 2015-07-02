#include "stdafx.h"
#include "Chunk.h"

#include "VoxelSpaceConversions.h"

ui32 Chunk::vboIndicesID = 0;

void Chunk::init(ChunkID id, const ChunkPosition3D& pos) {
    memset(neighbors, 0, sizeof(neighbors));  
    numNeighbors = 0u;
    distance2 = FLT_MAX;
    m_chunkPosition = pos;
    m_voxelPosition = VoxelSpaceConversions::chunkToVoxel(m_chunkPosition);
    refCount = 0;
    m_genQueryData.current = nullptr;
    remeshFlags = 1;
    // Maybe do this after its done being used in grid?
    std::vector<ChunkQuery*>().swap(m_genQueryData.pending);
    id = id;
    numBlocks = 0;
    hasCreatedMesh = false;
}

void Chunk::setRecyclers(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler) {
    blocks.setArrayRecycler(shortRecycler);
    tertiary.setArrayRecycler(shortRecycler);
}

void Chunk::updateContainers() {
    blocks.update(mutex);
    tertiary.update(mutex);
}