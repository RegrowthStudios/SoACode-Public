#include "stdafx.h"
#include "NChunk.h"

void NChunk::init(const ChunkPosition3D& pos) {
    memset(neighbors, 0, sizeof(neighbors));
}

void NChunk::set(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler,
         vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8>* byteRecycler) {
    m_blocks.setArrayRecycler(shortRecycler);
    m_sunlight.setArrayRecycler(byteRecycler);
    m_lamp.setArrayRecycler(shortRecycler);
    m_tertiary.setArrayRecycler(shortRecycler);
}