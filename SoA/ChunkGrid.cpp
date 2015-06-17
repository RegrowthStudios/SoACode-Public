#include "stdafx.h"
#include "ChunkGrid.h"

#include "ChunkMemoryManager.h"

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

const i16* ChunkGrid::getIDQuery(const i32v3& start, const i32v3& end) const {
    i32v3 pIter = start;
    // Create The Array For The IDs
    const i32v3 size = end - start + i32v3(1);
    i32 volume = size.x * size.y * size.z;
    i16* q = new i16[volume];
    i32 i = 0;
    i32v3 chunkPos, voxelPos;
    for (; pIter.y <= end.y; pIter.y++) {
        for (pIter.z = start.z; pIter.z <= end.z; pIter.z++) {
            for (pIter.x = start.x; pIter.x <= end.x; pIter.x++) {
                // Get The Chunk
                chunkPos = pIter / CHUNK_WIDTH;
                const Chunk* c = getChunk(chunkPos);
                // Get The ID
                voxelPos = pIter % CHUNK_WIDTH;
                q[i++] = c->getBlockID(voxelPos.y * CHUNK_LAYER + voxelPos.z * CHUNK_WIDTH + voxelPos.x);
            }
        }
    }
    // openglManager.debugRenderer->drawCube(
    // f32v3(start + end) * 0.5f + f32v3(cornerPosition) + f32v3(0.5f), f32v3(size) + f32v3(0.4f),
    // f32v4(1.0f, 0.0f, 0.0f, 0.3f), 1.0f
    // );
    return q;
}
