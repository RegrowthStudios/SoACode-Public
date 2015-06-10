#include "stdafx.h"
#include "NChunkGrid.h"
#include "NChunk.h"
#include "ChunkAllocator.h"

#include <Vorb/utils.h>

void NChunkGrid::init(WorldCubeFace face, ChunkAllocator* chunkAllocator,
                      OPT vcore::ThreadPool<WorkerData>* threadPool,
                      ui32 generatorsPerRow,
                      PlanetGenData* genData) {
    m_face = face;
    m_generatorsPerRow = generatorsPerRow;
    m_numGenerators = generatorsPerRow * generatorsPerRow;
    m_generators = new ChunkGenerator[m_numGenerators]; // TODO(Ben): delete[]
    for (ui32 i = 0; i < m_numGenerators; i++) {
        m_generators[i].init(chunkAllocator, threadPool, genData);
    }
}

void NChunkGrid::addChunk(NChunk* chunk) {
    const ChunkPosition3D& pos = chunk->getPosition();
    // Add to lookup hashmap
    m_chunkMap[pos.pos] = chunk;
    // TODO(Ben): use the () thingy
    i32v2 gridPos(pos.pos.x, pos.pos.z);

    // Check and see if the grid data is already allocated here
    NChunkGridData* gridData = getChunkGridData(gridPos);
    if (gridData == nullptr) {
        // If its not allocated, make a new one with a new voxelMapData
        // TODO(Ben): Cache this
        gridData = new NChunkGridData(pos);
        m_chunkGridDataMap[gridPos] = gridData;
    } else {
        gridData->refCount++;
    }
    chunk->gridData = gridData;
}

void NChunkGrid::removeChunk(NChunk* chunk) {
    const ChunkPosition3D& pos = chunk->getPosition();
    // Remove from lookup hashmap
    m_chunkMap.erase(pos.pos);

    // Reduce the ref count since the chunk no longer needs chunkGridData
    chunk->gridData->refCount--;
    // Check to see if we should free the grid data
    if (chunk->gridData->refCount == 0) {
        i32v2 gridPosition(pos.pos.x, pos.pos.z);
        m_chunkGridDataMap.erase(gridPosition);
        delete chunk->gridData;
    }
}

NChunk* NChunkGrid::getChunk(const f64v3& position) {

    i32v3 chPos(fastFloor(position.x / (f64)CHUNK_WIDTH),
                fastFloor(position.y / (f64)CHUNK_WIDTH),
                fastFloor(position.z / (f64)CHUNK_WIDTH));

    auto it = m_chunkMap.find(chPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;
}

NChunk* NChunkGrid::getChunk(const i32v3& chunkPos) {
    auto it = m_chunkMap.find(chunkPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;
}

const NChunk* NChunkGrid::getChunk(const i32v3& chunkPos) const {
    auto it = m_chunkMap.find(chunkPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;
}

void NChunkGrid::submitQuery(ChunkQuery* query) {
    m_queries.enqueue(query);
}

NChunkGridData* NChunkGrid::getChunkGridData(const i32v2& gridPos) const {
    auto it = m_chunkGridDataMap.find(gridPos);
    if (it == m_chunkGridDataMap.end()) return nullptr;
    return it->second;
}

void NChunkGrid::update() {
    // TODO(Ben): Handle generator distribution
    m_generators[0].update();

#define MAX_QUERIES 100
    ChunkQuery* queries[MAX_QUERIES];
    size_t numQueries = m_queries.try_dequeue_bulk(queries, MAX_QUERIES);
    for (size_t i = 0; i < numQueries; i++) {
        ChunkQuery* q = queries[i];
        NChunk* chunk = getChunk(q->chunkPos);
        if (chunk) {
            // Check if we don't need to do any generation
            if (chunk->genLevel <= q->genLevel) {
                q->m_chunk = chunk;
                q->m_isFinished = true;
                q->m_cond.notify_one();
                continue;
            } else {
                q->m_chunk = chunk;
            }
        } else {
            q->m_chunk = m_allocator->getNewChunk();
            ChunkPosition3D pos;
            pos.pos = q->chunkPos;
            pos.face = m_face;
            q->m_chunk->init(pos);
            addChunk(q->m_chunk);
        }
        // TODO(Ben): Handle generator distribution
        m_generators[0].submitQuery(q);
    }
}