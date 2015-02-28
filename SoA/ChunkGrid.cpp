#include "stdafx.h"
#include "ChunkGrid.h"

#include "ChunkMemoryManager.h"


ChunkGrid::~ChunkGrid() {
    // Empty
}

void ChunkGrid::addChunk(Chunk* chunk) {
    // Add to lookup hashmap
    m_chunkMap[chunk->gridPosition.pos] = chunk;

    i32v2 gridPos(chunk->gridPosition.pos.x,
                  chunk->gridPosition.pos.z);

    // Check and see if the grid data is already allocated here
    std::shared_ptr<ChunkGridData> chunkGridData = getChunkGridData(gridPos);
    if (chunkGridData == nullptr) {
        // If its not allocated, make a new one with a new voxelMapData
        chunkGridData = std::make_shared<ChunkGridData>(gridPos,
                                                        chunk->gridPosition.face);
        m_chunkGridDataMap[gridPos] = chunkGridData;
    } else {
        chunkGridData->refCount++;
    }
    chunk->chunkGridData = chunkGridData;
}

void ChunkGrid::removeChunk(Chunk* chunk) {
    // Remove from lookup hashmap
    m_chunkMap.erase(chunk->gridPosition.pos);
    // Reduce the ref count since the chunk no longer needs chunkGridData
    chunk->chunkGridData->refCount--;
    // Check to see if we should free the grid data
    if (chunk->chunkGridData->refCount == 0) {
        i32v2 gridPosition(chunk->gridPosition.pos.x, chunk->gridPosition.pos.z);
        m_chunkGridDataMap.erase(gridPosition);
        chunk->chunkGridData.reset(); // TODO(Ben): is shared needed?
    }
}

Chunk* ChunkGrid::getChunk(const f64v3& position) {

    i32v3 chPos(fastFloor(position.x / (f64)CHUNK_WIDTH),
                fastFloor(position.y / (f64)CHUNK_WIDTH),
                fastFloor(position.z / (f64)CHUNK_WIDTH));

    auto it = m_chunkMap.find(chPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;
}

Chunk* ChunkGrid::getChunk(const i32v3& chunkPos) {
    auto it = m_chunkMap.find(chunkPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;
}

const Chunk* ChunkGrid::getChunk(const i32v3& chunkPos) const {
    auto it = m_chunkMap.find(chunkPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;
}
