#include "stdafx.h"
#include "ChunkGrid.h"
#include "Chunk.h"
#include "ChunkAllocator.h"
#include "soaUtils.h"

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
    m_accessor.onAdd += makeDelegate(*this, &ChunkGrid::onAccessorAdd);
    m_accessor.onRemove += makeDelegate(*this, &ChunkGrid::onAccessorRemove);
}

void ChunkGrid::dispose() {
    m_accessor.onAdd -= makeDelegate(*this, &ChunkGrid::onAccessorAdd);
    m_accessor.onRemove -= makeDelegate(*this, &ChunkGrid::onAccessorRemove);
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
        }

        // TODO(Ben): Handle generator distribution
        q->genTask.init(q, q->chunk->gridData->heightData, &m_generators[0]);
        m_generators[0].submitQuery(q);
    }

    // TODO(Ben): Thread safety
    // Sort chunks
   // std::sort(m_activeChunks.begin(), m_activeChunks.end(), chunkSort);
}

void ChunkGrid::connectNeighbors(ChunkHandle chunk) {
    { // Left
        ChunkID id = chunk->getID();
        id.x--;
        chunk->left = m_accessor.acquire(id);
    }
    { // Right
        ChunkID id = chunk->getID();
        id.x++;
        chunk->right = m_accessor.acquire(id);
    }
    { // Bottom
        ChunkID id = chunk->getID();
        id.y--;
        chunk->bottom = m_accessor.acquire(id);
    }
    { // Top
        ChunkID id = chunk->getID();
        id.y++;
        chunk->top = m_accessor.acquire(id);
    }
    { // Back
        ChunkID id = chunk->getID();
        id.z--;
        chunk->back = m_accessor.acquire(id);
    }
    { // Front
        ChunkID id = chunk->getID();
        id.z++;
        chunk->front = m_accessor.acquire(id);
    } 
}

void ChunkGrid::disconnectNeighbors(ChunkHandle chunk) {
    chunk->left.release();
    chunk->right.release();
    chunk->back.release();
    chunk->front.release();
    chunk->bottom.release();
    chunk->top.release();
}

void ChunkGrid::onAccessorAdd(Sender s, ChunkHandle chunk) {
    // Add to active list
    chunk->m_activeIndex = m_activeChunks.size();
    m_activeChunks.push_back(chunk);

    // Init the chunk
    chunk->init(m_face);

    i32v2 gridPos = chunk->getChunkPosition();
    /* Get grid data */
    // Check and see if the grid data is already allocated here
    chunk->gridData = getChunkGridData(gridPos);
    if (chunk->gridData == nullptr) {
        // If its not allocated, make a new one with a new voxelMapData
        // TODO(Ben): Cache this
        chunk->gridData = new ChunkGridData(chunk->getChunkPosition());
        m_chunkGridDataMap[gridPos] = chunk->gridData;
    } else {
        chunk->gridData->refCount++;
    }
    chunk->heightData = chunk->gridData->heightData;
}

void ChunkGrid::onAccessorRemove(Sender s, ChunkHandle chunk) {
    // Remove from active list
    m_activeChunks[chunk->m_activeIndex] = m_activeChunks.back();
    m_activeChunks[chunk->m_activeIndex]->m_activeIndex = chunk->m_activeIndex;
    m_activeChunks.pop_back();

    // Remove and possibly free grid data
    chunk->gridData->refCount--;
    if (chunk->gridData->refCount == 0) {
        m_chunkGridDataMap.erase(chunk->getChunkPosition());
        delete chunk->gridData;
        chunk->gridData = nullptr;
        chunk->heightData = nullptr;
    }
}