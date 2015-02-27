#include "stdafx.h"
#include "ChunkGrid.h"

#include "ChunkMemoryManager.h"


ChunkGrid::ChunkGrid(ChunkMemoryManager* chunkMemoryManager) :
    m_chunkMemoryManager(chunkMemoryManager) {
    // Empty
}

ChunkGrid::~ChunkGrid() {
}

void ChunkGrid::addChunk(Chunk* chunk) {
    m_chunkMap[chunk->gridPosition.pos] = chunk->getID();
}

void ChunkGrid::addChunk(ChunkID chunkID) {
    Chunk& chunk = m_chunkMemoryManager->getChunk(chunkID);
    m_chunkMap[chunk.gridPosition.pos] = chunk.getID();
}

void ChunkGrid::removeChunk(Chunk* chunk) {
    m_chunkMap.erase(chunk->gridPosition.pos);
}

void ChunkGrid::removeChunk(ChunkID chunkID) {
    Chunk& chunk = m_chunkMemoryManager->getChunk(chunkID);
    m_chunkMap.erase(chunk.gridPosition.pos);
}


Chunk* ChunkGrid::getChunk(const f64v3& position) {

    i32v3 chPos(fastFloor(position.x / (f64)CHUNK_WIDTH),
                fastFloor(position.y / (f64)CHUNK_WIDTH),
                fastFloor(position.z / (f64)CHUNK_WIDTH));

    auto it = m_chunkMap.find(chPos);
    if (it == m_chunkMap.end()) return nullptr;
    return &m_chunkMemoryManager->getChunk(it->second);
}

Chunk* ChunkGrid::getChunk(const i32v3& chunkPos) {
    auto it = m_chunkMap.find(chunkPos);
    if (it == m_chunkMap.end()) return nullptr;
    return &m_chunkMemoryManager->getChunk(it->second);
}

const Chunk* ChunkGrid::getChunk(const i32v3& chunkPos) const {
    auto it = m_chunkMap.find(chunkPos);
    if (it == m_chunkMap.end()) return nullptr;
    return &m_chunkMemoryManager->getChunk(it->second);
}