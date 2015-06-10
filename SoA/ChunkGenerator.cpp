#include "stdafx.h"
#include "ChunkGenerator.h"

#include "ChunkAllocator.h"
#include "NChunk.h"

void ChunkGenerator::init(ChunkAllocator* chunkAllocator,
                          vcore::ThreadPool<WorkerData>* threadPool,
                          PlanetGenData* genData) {
    m_allocator = chunkAllocator;
    m_threadPool = threadPool;
    m_proceduralGenerator.init(genData);
}

void ChunkGenerator::submitQuery(ChunkQuery* query) {
    NChunk* chunk = query->getChunk();
    if (chunk->m_currentGenQuery) {
        // Only one gen query should be active at a time so just store this one
        chunk->m_pendingGenQueries.push_back(query);
    } else {
        // Submit for generation
        chunk->m_currentGenQuery = query;
        m_threadPool->addTask(&query->genTask);
    }
}

void ChunkGenerator::onQueryFinish(ChunkQuery* query) {
    m_finishedQueries.enqueue(query);
}

// Updates finished queries
void ChunkGenerator::update() {
#define MAX_QUERIES 100
    ChunkQuery* queries[MAX_QUERIES];
    size_t numQueries = m_finishedQueries.try_dequeue_bulk(queries, MAX_QUERIES);
    for (size_t i = 0; i < numQueries; i++) {
        NChunk* chunk = queries[i]->getChunk();
        chunk->m_currentGenQuery = nullptr;
        if (chunk->genLevel == DONE) {
            // If the chunk is done generating, we can signal all queries as done.
            for (auto& it : chunk->m_pendingGenQueries) {
                it->m_isFinished = true;
                it->m_cond.notify_one();
            }
            chunk->m_pendingGenQueries.clear();
        } else {
            // Otherwise possibly only some queries are done
            for (size_t i = 0; i < chunk->m_pendingGenQueries.size();) {
                ChunkQuery* q = chunk->m_pendingGenQueries[i];
                if (q->genLevel <= chunk->genLevel) {
                    q->m_isFinished = true;
                    q->m_cond.notify_one();
                    // TODO(Ben): Do we care about order?
                    chunk->m_pendingGenQueries[i] = chunk->m_pendingGenQueries.back();
                    chunk->m_pendingGenQueries.pop_back();
                } else {
                    i++;
                }
            }
            // Submit a pending query
            if (chunk->m_pendingGenQueries.size()) {
                // Submit for generation
                ChunkQuery* q = chunk->m_pendingGenQueries.back();
                chunk->m_currentGenQuery = q;
                m_threadPool->addTask(&q->genTask);
                chunk->m_pendingGenQueries.pop_back();
            }
        }
    }
}