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
    delete[] m_chunkMemory;
}

void ChunkMemoryManager::setSize(size_t numChunks) {
    m_maxSize = numChunks;

    delete[] m_chunkMemory;
    m_chunkMemory = new Chunk[m_maxSize];
    m_activeChunks.reserve(m_maxSize);

    std::cout << "Allocating chunk array to " << sizeof(Chunk) * m_maxSize / 1024.0 / 1024.0 << " MB\n";

    for (int i = 0; i < m_maxSize; i++) {
        m_chunkMemory[i].set(i, &m_shortFixedSizeArrayRecycler,
                                 &m_byteFixedSizeArrayRecycler);
    }
    // Set free chunks list
    m_freeChunks.resize(m_maxSize);
    for (int i = 0; i < m_maxSize; i++) {
        // We reverse order since its a stack and we pull from the top
        m_freeChunks[i] = m_maxSize - i - 1;
    }
}

Chunk* ChunkMemoryManager::getNewChunk() {
    if (m_freeChunks.size()) {
        ChunkID id = m_freeChunks.back();
        m_freeChunks.pop_back();
        m_chunkMemory[id].m_iterIndex = m_activeChunks.size();
        m_activeChunks.push_back(&m_chunkMemory[id]);
        return &m_chunkMemory[id];
    }
    return nullptr;
}

void ChunkMemoryManager::freeChunk(Chunk* chunk) {
    chunk->_state = ChunkStates::INACTIVE;
    m_activeChunks[chunk->m_iterIndex] = m_activeChunks.back();
    m_activeChunks[chunk->m_iterIndex]->m_iterIndex = chunk->m_iterIndex;
    m_activeChunks.pop_back();
    chunk->m_iterIndex = -1;
    m_freeChunks.push_back(chunk->getID());
}
