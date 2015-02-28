#include "stdafx.h"
#include "ChunkMemoryManager.h"


void ChunkMemoryManager::setSize(size_t numChunks) {
    m_maxSize = numChunks;
    // Allocate memory
    m_chunkMemory.resize(m_maxSize);
    for (int i = 0; i < m_maxSize; i++) {
        m_chunkMemory[i] = Chunk(i, &m_shortFixedSizeArrayRecycler,
                                 &m_byteFixedSizeArrayRecycler);
    }
    // Set free chunks list
    m_freeChunks.resize(m_maxSize);
    for (int i = 0; i < m_maxSize; i++) {
        m_freeChunks[i] = i;
    }
}

Chunk* ChunkMemoryManager::getNewChunk() {
    if (m_freeChunks.size()) {
        ChunkID id = m_freeChunks.back();
        m_freeChunks.pop_back();
        return &m_chunkMemory[id];
    }
    return nullptr;
}

void ChunkMemoryManager::freeChunk(Chunk* chunk) {
    m_freeChunks.push_back(chunk->getID());
}
