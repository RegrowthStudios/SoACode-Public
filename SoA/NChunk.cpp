#include "stdafx.h"
#include "NChunk.h"

void NChunk::init(const ChunkPosition3D& pos) {
    memset(neighbors, 0, sizeof(neighbors));
    m_numNeighbors = 0u;
    distance2 = FLT_MAX;
}

void NChunk::setRecyclers(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler,
         vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8>* byteRecycler) {
    m_blocks.setArrayRecycler(shortRecycler);
    m_sunlight.setArrayRecycler(byteRecycler);
    m_lamp.setArrayRecycler(shortRecycler);
    m_tertiary.setArrayRecycler(shortRecycler);
}
