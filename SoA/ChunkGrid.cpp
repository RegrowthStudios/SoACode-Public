#include "stdafx.h"
#include "ChunkGrid.h"
#include "Chunk.h"
#include "ChunkAllocator.h"

#include <Vorb/utils.h>

volatile ChunkID ChunkGrid::m_nextAvailableID = 0;

void ChunkGrid::init(WorldCubeFace face,
                      OPT vcore::ThreadPool<WorkerData>* threadPool,
                      ui32 generatorsPerRow,
                      PlanetGenData* genData,
                      PagedChunkAllocator* allocator) {
    m_face = face;
    m_generatorsPerRow = generatorsPerRow;
    m_numGenerators = generatorsPerRow * generatorsPerRow;
    m_generators = new ChunkGenerator[m_numGenerators]; // TODO(Ben): delete[]
    for (ui32 i = 0; i < m_numGenerators; i++) {
        m_generators[i].init(threadPool, genData);
    }
    m_accessor.init(allocator);
}

void ChunkGrid::addChunk(Chunk* chunk) {
    const ChunkPosition3D& pos = chunk->getChunkPosition();
    // Add to lookup hashmap
    m_chunkMap[pos.pos] = chunk;
    // TODO(Ben): use the () thingy
    i32v2 gridPos(pos.pos.x, pos.pos.z);

    { // Get grid data
        // Check and see if the grid data is already allocated here
        chunk->gridData = getChunkGridData(gridPos);
        if (chunk->gridData == nullptr) {
            // If its not allocated, make a new one with a new voxelMapData
            // TODO(Ben): Cache this
            chunk->gridData = new ChunkGridData(pos);
            m_chunkGridDataMap[gridPos] = chunk->gridData;
        } else {
            chunk->gridData->refCount++;
        }
        chunk->heightData = chunk->gridData->heightData;
    }

    connectNeighbors(chunk);

    // Add to active list
    m_activeChunks.push_back(chunk);
}

void ChunkGrid::removeChunk(Chunk* chunk, int index) {
    const ChunkPosition3D& pos = chunk->getChunkPosition();

    // Remove from lookup hashmap
    m_chunkMap.erase(pos.pos);

    // Remove and possibly free grid data
    // TODO(Cristian): This needs to be moved
    chunk->gridData->refCount--;
    if (chunk->gridData->refCount == 0) {
        m_chunkGridDataMap.erase(i32v2(pos.pos.x, pos.pos.z));
        delete chunk->gridData;
        chunk->gridData = nullptr;
        chunk->heightData = nullptr;
    }
    
    disconnectNeighbors(chunk);

    // Remove from active list
    m_activeChunks[index] = m_activeChunks.back();
    m_activeChunks.pop_back();

    // Free the chunk
    m_allocator.free(chunk);
}

ChunkHandle ChunkGrid::getChunk(const f64v3& position) {

    i32v3 chPos(fastFloor(position.x / (f64)CHUNK_WIDTH),
                fastFloor(position.y / (f64)CHUNK_WIDTH),
                fastFloor(position.z / (f64)CHUNK_WIDTH));

    auto it = m_chunkMap.find(chPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;
}

ChunkHandle ChunkGrid::getChunk(const i32v3& chunkPos) {
    auto it = m_chunkMap.find(chunkPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;
}

const ChunkHandle ChunkGrid::getChunk(const i32v3& chunkPos) const {
    auto it = m_chunkMap.find(chunkPos);
    if (it == m_chunkMap.end()) return nullptr;
    return it->second;
}

void ChunkGrid::submitQuery(ChunkQuery* query) {
    m_queries.enqueue(query);
}

ChunkGridData* ChunkGrid::getChunkGridData(const i32v2& gridPos) const {
    auto it = m_chunkGridDataMap.find(gridPos);
    if (it == m_chunkGridDataMap.end()) return nullptr;
    return it->second;
}

bool chunkSort(const Chunk* a, const Chunk* b) {
    return a->distance2 > b->distance2;
}

void ChunkGrid::update() {
    // TODO(Ben): Handle generator distribution
    m_generators[0].update();

    // Needs to be big so we can flush it every frame.
#define MAX_QUERIES 5000
    ChunkQuery* queries[MAX_QUERIES];
    size_t numQueries = m_queries.try_dequeue_bulk(queries, MAX_QUERIES);
    for (size_t i = 0; i < numQueries; i++) {
        ChunkQuery* q = queries[i];
        Chunk* chunk = getChunk(q->chunkPos);
        if (chunk) {
            // Check if we don't need to do any generation
            if (chunk->genLevel <= q->genLevel) {
                q->m_chunk = chunk;
                q->m_isFinished = true;
                q->m_cond.notify_one();
                if (q->shouldDelete) delete q;
                continue;
            } else {
                q->m_chunk = chunk;
                q->m_chunk->refCount++;
            }
        } else {
            q->m_chunk = m_allocator.alloc();
            ChunkPosition3D pos;
            pos.pos = q->chunkPos;
            pos.face = m_face;
            q->m_chunk->init(m_nextAvailableID++, pos);
            q->m_chunk->refCount++;
            addChunk(q->m_chunk);
        }
        // TODO(Ben): Handle generator distribution
        q->genTask.init(q, q->m_chunk->gridData->heightData, &m_generators[0]);
        m_generators[0].submitQuery(q);
    }

    // Sort chunks
    std::sort(m_activeChunks.begin(), m_activeChunks.end(), chunkSort);
}

void ChunkGrid::connectNeighbors(Chunk* chunk) {
    const i32v3& pos = chunk->getChunkPosition().pos;
    { // Left
        i32v3 newPos(pos.x - 1, pos.y, pos.z);
        chunk->left = getChunk(newPos);
        if (chunk->left) {
            chunk->left->right = chunk;
            chunk->left->numNeighbors++;
            chunk->numNeighbors++;
        }
    }
    { // Right
        i32v3 newPos(pos.x + 1, pos.y, pos.z);
        chunk->right = getChunk(newPos);
        if (chunk->right) {
            chunk->right->left = chunk;
            chunk->right->numNeighbors++;
            chunk->numNeighbors++;
        }
    }
    { // Bottom
        i32v3 newPos(pos.x, pos.y - 1, pos.z);
        chunk->bottom = getChunk(newPos);
        if (chunk->bottom) {
            chunk->bottom->top = chunk;
            chunk->bottom->numNeighbors++;
            chunk->numNeighbors++;
        }
    }
    { // Top
        i32v3 newPos(pos.x, pos.y + 1, pos.z);
        chunk->top = getChunk(newPos);
        if (chunk->top) {
            chunk->top->bottom = chunk;
            chunk->top->numNeighbors++;
            chunk->numNeighbors++;
        }
    }
    { // Back
        i32v3 newPos(pos.x, pos.y, pos.z - 1);
        chunk->back = getChunk(newPos);
        if (chunk->back) {
            chunk->back->front = chunk;
            chunk->back->numNeighbors++;
            chunk->numNeighbors++;
        }
    }
    { // Front
        i32v3 newPos(pos.x, pos.y, pos.z + 1);
        chunk->front = getChunk(newPos);
        if (chunk->front) {
            chunk->front->back = chunk;
            chunk->front->numNeighbors++;
            chunk->numNeighbors++;
        }
    } 
}

void ChunkGrid::disconnectNeighbors(Chunk* chunk) {
    if (chunk->left) {
        chunk->left->right = nullptr;
        chunk->left->numNeighbors--;
    }
    if (chunk->right) {
        chunk->right->left = nullptr;
        chunk->right->numNeighbors--;
    }
    if (chunk->bottom) {
        chunk->bottom->top = nullptr;
        chunk->bottom->numNeighbors--;
    }
    if (chunk->top) {
        chunk->top->bottom = nullptr;
        chunk->top->numNeighbors--;
    }
    if (chunk->back) {
        chunk->back->front = nullptr;
        chunk->back->numNeighbors--;
    }
    if (chunk->front) {
        chunk->front->back = nullptr;
        chunk->front->numNeighbors--;
    }
    memset(chunk->neighbors, 0, sizeof(chunk->neighbors));
    chunk->numNeighbors = 0;
}