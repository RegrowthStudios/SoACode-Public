#include "stdafx.h"
#include "NChunk.h"

#include "VoxelSpaceConversions.h"

ui32 NChunk::vboIndicesID = 0;

void NChunk::init(const ChunkPosition3D& pos) {
    memset(neighbors, 0, sizeof(neighbors));  
    m_numNeighbors = 0u;
    m_distance2 = FLT_MAX;
    m_chunkPosition = pos;
    m_voxelPosition = VoxelSpaceConversions::chunkToVoxel(m_chunkPosition);
    refCount = 0;
    m_genQueryData.current = nullptr;
    m_remeshFlags = 1;
    m_genQueryData.pending.clear();
}

void NChunk::setRecyclers(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler,
         vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8>* byteRecycler) {
    m_blocks.setArrayRecycler(shortRecycler);
    m_sunlight.setArrayRecycler(byteRecycler);
    m_lamp.setArrayRecycler(shortRecycler);
    m_tertiary.setArrayRecycler(shortRecycler);
}

void NChunk::updateContainers() {
    m_blocks.update(mutex);
    m_sunlight.update(mutex);
    m_lamp.update(mutex);
    m_tertiary.update(mutex);
}