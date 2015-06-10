#include "stdafx.h"
#include "ChunkGenerator.h"

#include "ChunkAllocator.h"

void ChunkGenerator::init(ChunkAllocator* chunkAllocator) {
    m_allocator = chunkAllocator;
}