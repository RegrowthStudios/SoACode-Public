#include "stdafx.h"
#include "Chunk.h"

#include "VoxelSpaceConversions.h"

ui32 Chunk::vboIndicesID = 0;

void Chunk::init(ChunkID id, const ChunkPosition3D& pos) {
    memset(neighbors, 0, sizeof(neighbors));  
    m_numNeighbors = 0u;
    m_distance2 = FLT_MAX;
    m_chunkPosition = pos;
    m_voxelPosition = VoxelSpaceConversions::chunkToVoxel(m_chunkPosition);
    refCount = 0;
    m_genQueryData.current = nullptr;
    m_remeshFlags = 1;
    // Maybe do this after its done being used in grid?
    std::vector<ChunkQuery*>().swap(m_genQueryData.pending);
    m_id = id;
    numBlocks = 0;
    hasCreatedMesh = false;
}

void Chunk::setRecyclers(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler) {
    m_blocks.setArrayRecycler(shortRecycler);
    m_tertiary.setArrayRecycler(shortRecycler);
}

void Chunk::updateContainers() {
    m_blocks.update(mutex);
    m_tertiary.update(mutex);
}