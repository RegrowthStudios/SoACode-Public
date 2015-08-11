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
    generatorsPerRow = generatorsPerRow;
    numGenerators = generatorsPerRow * generatorsPerRow;
    generators = new ChunkGenerator[numGenerators];
    for (ui32 i = 0; i < numGenerators; i++) {
        generators[i].init(threadPool, genData);
    }
    accessor.init(allocator);
    accessor.onAdd += makeDelegate(*this, &ChunkGrid::onAccessorAdd);
    accessor.onRemove += makeDelegate(*this, &ChunkGrid::onAccessorRemove);
}

void ChunkGrid::dispose() {
    accessor.onAdd -= makeDelegate(*this, &ChunkGrid::onAccessorAdd);
    accessor.onRemove -= makeDelegate(*this, &ChunkGrid::onAccessorRemove);
    delete[] generators;
    generators = nullptr;
}

ChunkQuery* ChunkGrid::submitQuery(const i32v3& chunkPos, ChunkGenLevel genLevel, bool shouldRelease) {
    ChunkQuery* query;
    {
        std::lock_guard<std::mutex> l(m_lckQueryRecycler);
        query = m_queryRecycler.create();
    }
    query->chunkPos = chunkPos;
    query->genLevel = genLevel;
    query->shouldRelease = shouldRelease;
    query->m_grid = this;
    query->m_isFinished = false;

    ChunkID id(query->chunkPos);
    query->chunk = accessor.acquire(id);
    m_queries.enqueue(query);
    // TODO(Ben): RACE CONDITION HERE: There is actually a very small chance that
    // chunk will get freed before the callee can acquire the chunk, if this runs
    // on another thread.
    return query;
}

void ChunkGrid::releaseQuery(ChunkQuery* query) {
    assert(query->m_grid);
    query->m_grid = nullptr;
    {
        std::lock_guard<std::mutex> l(m_lckQueryRecycler);
        m_queryRecycler.recycle(query);
    }
}

ChunkGridData* ChunkGrid::getChunkGridData(const i32v2& gridPos) {
    std::lock_guard<std::mutex> l(m_lckGridData);
    auto it = m_chunkGridDataMap.find(gridPos);
    if (it == m_chunkGridDataMap.end()) return nullptr;
    return it->second;
}

void ChunkGrid::update() {
    std::vector<ChunkHandle> toRelease;
    { // Update active list
        // Have to use a single vector so order is consistent
        std::lock_guard<std::mutex> l(m_lckAddOrRemove);
        for (auto& it : m_activeChunksToAddOrRemove) {
            ChunkHandle& h = it.first;
            if (it.second) {
                // Add case
                h->m_activeIndex = m_activeChunks.size();
                m_activeChunks.push_back(h);
                toRelease.push_back(h);
            } else {
                // Remove case
                m_activeChunks[h->m_activeIndex] = m_activeChunks.back();
                m_activeChunks[h->m_activeIndex]->m_activeIndex = h->m_activeIndex;
                m_activeChunks.pop_back();
            }
        }
        m_activeChunksToAddOrRemove.clear();
    }
    for (auto& h : toRelease) {
        h.release();
    }

    // TODO(Ben): Handle generator distribution
    generators[0].update();

    /* Update Queries */
    // Needs to be big so we can flush it every frame.
#define MAX_QUERIES 5000
    ChunkQuery* queries[MAX_QUERIES];
    size_t numQueries = m_queries.try_dequeue_bulk(queries, MAX_QUERIES);
    for (size_t i = 0; i < numQueries; i++) {
        ChunkQuery* q = queries[i];
        // TODO(Ben): Handle generator distribution
        q->genTask.init(q, q->chunk->gridData->heightData, &generators[0]);
        generators[0].submitQuery(q);
    }
}

void ChunkGrid::onAccessorAdd(Sender s, ChunkHandle chunk) {
    { // Add to active list
        std::lock_guard<std::mutex> l(m_lckAddOrRemove);
        chunk.acquire();
        m_activeChunksToAddOrRemove.emplace_back(chunk, true);
    }

    // Init the chunk
    chunk->init(m_face);

    i32v2 gridPos = chunk->getChunkPosition();

    { // Get grid data
        std::lock_guard<std::mutex> l(m_lckGridData);
        // Check and see if the grid data is already allocated here
        auto it = m_chunkGridDataMap.find(gridPos);
        if (it == m_chunkGridDataMap.end()) {
            // If its not allocated, make a new one with a new voxelMapData
            // TODO(Ben): Cache this
            chunk->gridData = new ChunkGridData(chunk->getChunkPosition());
            m_chunkGridDataMap[gridPos] = chunk->gridData;
        } else {
            chunk->gridData = it->second;
            chunk->gridData->refCount++;
        }
    }
    chunk->heightData = chunk->gridData->heightData;
}

void ChunkGrid::onAccessorRemove(Sender s, ChunkHandle chunk) {
    { // Remove from active list
        std::lock_guard<std::mutex> l(m_lckAddOrRemove);
        m_activeChunksToAddOrRemove.emplace_back(chunk, false);
    }

    // TODO(Ben): Could be slightly faster with InterlockedDecrement and FSM?
    { // Remove and possibly free grid data
        std::unique_lock<std::mutex> l(m_lckGridData);
        chunk->gridData->refCount--;
        if (chunk->gridData->refCount == 0) {
            m_chunkGridDataMap.erase(chunk->getChunkPosition());
            l.unlock();
            delete chunk->gridData;
            chunk->gridData = nullptr;
            chunk->heightData = nullptr;
        }
    }
}
