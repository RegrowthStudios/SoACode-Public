#include "stdafx.h"
#include "ChunkMemoryManager.h"

#define MAX_VOXEL_ARRAYS_TO_CACHE 200
#define NUM_SHORT_VOXEL_ARRAYS 3
#define NUM_BYTE_VOXEL_ARRAYS 1

ChunkMemoryManager::ChunkMemoryManager() :
    m_shortFixedSizeArrayRecycler(MAX_VOXEL_ARRAYS_TO_CACHE * NUM_SHORT_VOXEL_ARRAYS),
    m_byteFixedSizeArrayRecycler(MAX_VOXEL_ARRAYS_TO_CACHE * NUM_BYTE_VOXEL_ARRAYS) {
    // Empty
}

ChunkMemoryManager::~ChunkMemoryManager() {
    for (auto& page : m_chunkPages) {
        delete page;
    }
}

Chunk* ChunkMemoryManager::getNewChunk() {
    // TODO(Ben): limit
    // Allocate chunk pages if needed
    if (m_freeChunks.empty()) {
        ChunkPage* page = new ChunkPage();
        m_chunkPages.push_back(page);
        // Add chunks to free chunks lists
        for (int i = 0; i < CHUNK_PAGE_SIZE; i++) {
            Chunk* chunk = &page->chunks[CHUNK_PAGE_SIZE - i - 1];
            chunk->set(&m_shortFixedSizeArrayRecycler, &m_byteFixedSizeArrayRecycler);
            m_freeChunks.push_back(chunk);
        }
    }
    // Grab a free chunk
    Chunk* chunk = m_freeChunks.back();
    m_freeChunks.pop_back();
    chunk->m_iterIndex = m_activeChunks.size();
    m_activeChunks.push_back(chunk);
    return chunk;
}

void ChunkMemoryManager::freeChunk(Chunk* chunk) {
    chunk->_state = ChunkStates::INACTIVE;
    m_activeChunks[chunk->m_iterIndex] = m_activeChunks.back();
    m_activeChunks[chunk->m_iterIndex]->m_iterIndex = chunk->m_iterIndex;
    m_activeChunks.pop_back();
    chunk->m_iterIndex = -1;
    m_freeChunks.push_back(chunk);
}
