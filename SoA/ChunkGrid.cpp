#include "stdafx.h"
#include "ChunkGrid.h"
#include "Chunk.h"
#include "ChunkAllocator.h"

#include <Vorb/utils.h>

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

void ChunkGrid::addChunk(ChunkHandle chunk) {
    chunk.acquire();

    const ChunkPosition3D& pos = chunk->getChunkPosition();

    i32v2 gridPos(pos.x, pos.z);

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

void ChunkGrid::removeChunk(ChunkHandle chunk, int index) {
    const ChunkPosition3D& pos = chunk->getChunkPosition();

    // Remove and possibly free grid data
    // TODO(Cristian): This needs to be moved
    chunk->gridData->refCount--;
    if (chunk->gridData->refCount == 0) {
        m_chunkGridDataMap.erase(i32v2(pos.x, pos.z));
        delete chunk->gridData;
        chunk->gridData = nullptr;
        chunk->heightData = nullptr;
    }
    
    disconnectNeighbors(chunk);

    // Remove from active list
    m_activeChunks[index] = m_activeChunks.back();
    m_activeChunks.pop_back();

    // Release the chunk ownership
    chunk.release();
}

ChunkHandle ChunkGrid::getChunk(const f64v3& voxelPos) {
    ChunkID id(fastFloor(voxelPos.x / (f64)CHUNK_WIDTH),
               fastFloor(voxelPos.y / (f64)CHUNK_WIDTH),
               fastFloor(voxelPos.z / (f64)CHUNK_WIDTH));
    return m_accessor.acquire(id);
}

ChunkHandle ChunkGrid::getChunk(const i32v3& chunkPos) {
    return m_accessor.acquire(ChunkID(chunkPos.x, chunkPos.y, chunkPos.z));
}

void ChunkGrid::submitQuery(ChunkQuery* query) {
    ChunkID id(query->chunkPos.x, query->chunkPos.y, query->chunkPos.z);
    query->chunk = m_accessor.acquire(id);
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
        if (q->isFinished()) {
            // Check if we don't need to do any generation
            if (q->chunk->genLevel <= q->genLevel) {
                q->m_isFinished = true;
                q->m_cond.notify_one();
                ChunkHandle chunk = q->chunk;
                chunk.release();
                if (q->shouldDelete) delete q;
                continue;
            }
        } else if (!q->chunk->m_genQueryData.current) {
            // If its not in a query, it needs an init
            ChunkPosition3D pos;
            pos.pos = q->chunkPos;
            pos.face = m_face;
            q->chunk->init(pos);
            addChunk(q->chunk);
        }
        // TODO(Ben): Handle generator distribution
        q->genTask.init(q, q->chunk->gridData->heightData, &m_generators[0]);
        m_generators[0].submitQuery(q);
    }

    // Sort chunks
    std::sort(m_activeChunks.begin(), m_activeChunks.end(), chunkSort);
}

void ChunkGrid::connectNeighbors(ChunkHandle chunk) {
    const i32v3& pos = chunk->getChunkPosition().pos;
    { // Left
        ChunkID id = chunk->getID();
        id.x--;
        chunk->left = m_accessor.acquire(id);
        chunk->left->right = chunk.acquire();
        chunk->left->numNeighbors++;
        chunk->numNeighbors++;
    }
    { // Right
        ChunkID id = chunk->getID();
        id.x++;
        chunk->right = m_accessor.acquire(id);
        chunk->right->left = chunk.acquire();
        chunk->right->numNeighbors++;
        chunk->numNeighbors++;
    }
    { // Bottom
        ChunkID id = chunk->getID();
        id.y--;
        chunk->bottom = m_accessor.acquire(id);
        chunk->bottom->top = chunk.acquire();
        chunk->bottom->numNeighbors++;
        chunk->numNeighbors++;
    }
    { // Top
        ChunkID id = chunk->getID();
        id.y++;
        chunk->top = m_accessor.acquire(id);
        chunk->top->bottom = chunk.acquire();
        chunk->top->numNeighbors++;
        chunk->numNeighbors++;
    }
    { // Back
        ChunkID id = chunk->getID();
        id.z--;
        chunk->back = m_accessor.acquire(id);
        chunk->back->front = chunk.acquire();
        chunk->back->numNeighbors++;
        chunk->numNeighbors++;
    }
    { // Front
        ChunkID id = chunk->getID();
        id.z++;
        chunk->front = m_accessor.acquire(id);
        chunk->front->back = chunk.acquire();
        chunk->front->numNeighbors++;
        chunk->numNeighbors++;
    } 
}

void ChunkGrid::disconnectNeighbors(ChunkHandle chunk) {
    if (chunk->left.isAquired()) {
        chunk->left->right.release();
        chunk->left->numNeighbors--;
        chunk->left.release();
    }
    if (chunk->right.isAquired()) {
        chunk->right->left.release();
        chunk->right->numNeighbors--;
        chunk->right.release();
    }
    if (chunk->bottom.isAquired()) {
        chunk->bottom->top.release();
        chunk->bottom->numNeighbors--;
        chunk->bottom.release();
    }
    if (chunk->top.isAquired()) {
        chunk->top->bottom.release();
        chunk->top->numNeighbors--;
        chunk->top.release();
    }
    if (chunk->back.isAquired()) {
        chunk->back->front.release();
        chunk->back->numNeighbors--;
        chunk->back.release();
    }
    if (chunk->front.isAquired()) {
        chunk->front->back.release();
        chunk->front->numNeighbors--;
        chunk->front.release();
    }
    
    chunk->numNeighbors = 0;
}