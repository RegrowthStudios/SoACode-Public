#include "stdafx.h"
#include "ChunkAllocator.h"
#include "Chunk.h"

#define MAX_VOXEL_ARRAYS_TO_CACHE 200
#define NUM_SHORT_VOXEL_ARRAYS 3
#define NUM_BYTE_VOXEL_ARRAYS 1

PagedChunkAllocator::PagedChunkAllocator() :
m_shortFixedSizeArrayRecycler(MAX_VOXEL_ARRAYS_TO_CACHE * NUM_SHORT_VOXEL_ARRAYS) {
    // Empty
}

PagedChunkAllocator::~PagedChunkAllocator() {
    for (auto& page : m_chunkPages) {
        delete page;
    }
}

Chunk* PagedChunkAllocator::alloc() {
    // TODO(Ben): limit
    std::lock_guard<std::mutex> lock(m_lock);

    // Allocate chunk pages if needed
    if (m_freeChunks.empty()) {
        ChunkPage* page = new ChunkPage();
        m_chunkPages.push_back(page);
        // Add chunks to free chunks lists
        for (int i = 0; i < CHUNK_PAGE_SIZE; i++) {
            Chunk* chunk = &page->chunks[CHUNK_PAGE_SIZE - i - 1];
            chunk->setRecyclers(&m_shortFixedSizeArrayRecycler);
            m_freeChunks.push_back(chunk);
        }
    }
    // Grab a free chunk
    Chunk* chunk = m_freeChunks.back();
    m_freeChunks.pop_back();
    return chunk;
}

void PagedChunkAllocator::free(Chunk* chunk) {
    // TODO(Ben): Deletion if there is a lot?
    std::lock_guard<std::mutex> lock(m_lock);
    m_freeChunks.push_back(chunk);
}
