#include "stdafx.h"
#include "ChunkMemoryManager.h"

void ChunkMemoryManager::reserve(int numChunks) {
    m_chunkMemory.reserve(numChunks);
}

ChunkID ChunkMemoryManager::getNewChunk() {
    if (m_freeChunks.size()) {
        ChunkID id = m_freeChunks.back();
        m_freeChunks.pop_back();
        return id;
    }
    m_chunkMemory.emplace_back(m_shortFixedSizeArrayRecycler,
                               m_byteFixedSizeArrayRecycler);
    return m_chunkMemory.size() - 1;
}

void ChunkMemoryManager::freeChunk(ChunkID chunkID) {
    m_freeChunks.push_back(chunkID);
}
