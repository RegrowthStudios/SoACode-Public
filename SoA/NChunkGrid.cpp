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
    m_allocator = chunkAllocator;
    m_generatorsPerRow = generatorsPerRow;
    m_numGenerators = generatorsPerRow * generatorsPerRow;
    m_generators = new ChunkGenerator[m_numGenerators]; // TODO(Ben): delete[]
    for (ui32 i = 0; i < m_numGenerators; i++) {
        m_generators[i].init(chunkAllocator, threadPool, genData);
    }
}

void NChunkGrid::addChunk(NChunk* chunk) {
    const ChunkPosition3D& pos = chunk->getChunkPosition();
    // Add to lookup hashmap
    m_chunkMap[pos.pos] = chunk;
    // TODO(Ben): use the () thingy
    i32v2 gridPos(pos.pos.x, pos.pos.z);

    { // Get grid data
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

    connectNeighbors(chunk);

    { // Add to linked list
        if (m_activeChunks) {
            chunk->m_nextActive = m_activeChunks;
            m_activeChunks->m_prevActive = chunk;
            chunk->m_prevActive = nullptr;
            m_activeChunks = chunk;
        } else {
            chunk->m_nextActive = nullptr;
            chunk->m_prevActive = nullptr;
            m_activeChunks = chunk;
        }
        m_numActiveChunks++;
    }
}

void NChunkGrid::removeChunk(NChunk* chunk) {
    const ChunkPosition3D& pos = chunk->getChunkPosition();
    // Remove from lookup hashmap
    m_chunkMap.erase(pos.pos);

    { // Remove grid data
        chunk->gridData->refCount--;
        // Check to see if we should free the grid data
        if (chunk->gridData->refCount == 0) {
            i32v2 gridPosition(pos.pos.x, pos.pos.z);
            m_chunkGridDataMap.erase(gridPosition);
            delete chunk->gridData;
        }
    }

    disconnectNeighbors(chunk);

    { // Remove from linked list
        if (chunk != m_activeChunks) {
            chunk->m_prevActive->m_nextActive = chunk->m_nextActive;
            chunk->m_nextActive->m_prevActive = chunk->m_prevActive;
        } else {
            m_activeChunks = chunk->m_nextActive;
            if (m_activeChunks) m_activeChunks->m_prevActive = nullptr;
        }
        m_numActiveChunks--;
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
                if (q->shouldDelete) delete q;
                continue;
            } else {
                q->m_chunk = chunk;
                q->m_chunk->refCount++;
            }
        } else {
            q->m_chunk = m_allocator->getNewChunk();
            ChunkPosition3D pos;
            pos.pos = q->chunkPos;
            pos.face = m_face;
            q->m_chunk->init(pos);
            q->m_chunk->refCount++;
            addChunk(q->m_chunk);
        }
        // TODO(Ben): Handle generator distribution
        q->genTask.init(q, q->m_chunk->gridData->heightData, &m_generators[0]);
        m_generators[0].submitQuery(q);
    }
}

void NChunkGrid::connectNeighbors(NChunk* chunk) {
    const i32v3& pos = chunk->getChunkPosition().pos;
    { // Left
        i32v3 newPos(pos.x - 1, pos.y, pos.z);
        chunk->left = getChunk(newPos);
        if (chunk->left) {
            chunk->left->right = chunk;
            chunk->left->m_numNeighbors++;
            chunk->m_numNeighbors++;
        }
    }
    { // Right
        i32v3 newPos(pos.x + 1, pos.y, pos.z);
        chunk->right = getChunk(newPos);
        if (chunk->right) {
            chunk->right->left = chunk;
            chunk->right->m_numNeighbors++;
            chunk->m_numNeighbors++;
        }
    }
    { // Bottom
        i32v3 newPos(pos.x, pos.y - 1, pos.z);
        chunk->bottom = getChunk(newPos);
        if (chunk->bottom) {
            chunk->bottom->top = chunk;
            chunk->bottom->m_numNeighbors++;
            chunk->m_numNeighbors++;
        }
    }
    { // Top
        i32v3 newPos(pos.x, pos.y + 1, pos.z);
        chunk->top = getChunk(newPos);
        if (chunk->top) {
            chunk->top->bottom = chunk;
            chunk->top->m_numNeighbors++;
            chunk->m_numNeighbors++;
        }
    }
    { // Back
        i32v3 newPos(pos.x, pos.y, pos.z - 1);
        chunk->back = getChunk(newPos);
        if (chunk->back) {
            chunk->back->front = chunk;
            chunk->back->m_numNeighbors++;
            chunk->m_numNeighbors++;
        }
    }
    { // Front
        i32v3 newPos(pos.x, pos.y, pos.z + 1);
        chunk->front = getChunk(newPos);
        if (chunk->front) {
            chunk->front->back = chunk;
            chunk->front->m_numNeighbors++;
            chunk->m_numNeighbors++;
        }
    } 
}

void NChunkGrid::disconnectNeighbors(NChunk* chunk) {
    if (chunk->left) {
        chunk->left->right = nullptr;
        chunk->left->m_numNeighbors--;
    }
    if (chunk->right) {
        chunk->right->left = nullptr;
        chunk->right->m_numNeighbors--;
    }
    if (chunk->bottom) {
        chunk->bottom->top = nullptr;
        chunk->bottom->m_numNeighbors--;
    }
    if (chunk->top) {
        chunk->top->bottom = nullptr;
        chunk->top->m_numNeighbors--;
    }
    if (chunk->back) {
        chunk->back->front = nullptr;
        chunk->back->m_numNeighbors--;
    }
    if (chunk->front) {
        chunk->front->back = nullptr;
        chunk->front->m_numNeighbors--;
    }
    memset(chunk->neighbors, 0, sizeof(chunk->neighbors));
    chunk->m_numNeighbors = 0;
}