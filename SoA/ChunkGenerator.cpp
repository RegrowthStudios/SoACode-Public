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
    if (chunk->m_genQueryData.current) {
        // Only one gen query should be active at a time so just store this one
        chunk->m_genQueryData.pending.push_back(query);
    } else {
        // Submit for generation
        chunk->m_genQueryData.current = query;
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
        chunk->m_genQueryData.current = nullptr;
        std::cout << "YAY";
        if (chunk->genLevel == GEN_DONE) {
            // If the chunk is done generating, we can signal all queries as done.
            for (auto& it : chunk->m_genQueryData.pending) {
                it->m_isFinished = true;
                it->m_cond.notify_one();
            }
            chunk->m_genQueryData.pending.clear();
        } else {
            // Otherwise possibly only some queries are done
            for (size_t i = 0; i < chunk->m_genQueryData.pending.size();) {
                ChunkQuery* q = chunk->m_genQueryData.pending[i];
                if (q->genLevel <= chunk->genLevel) {
                    q->m_isFinished = true;
                    q->m_cond.notify_one();
                    // TODO(Ben): Do we care about order?
                    chunk->m_genQueryData.pending[i] = chunk->m_genQueryData.pending.back();
                    chunk->m_genQueryData.pending.pop_back();
                } else {
                    i++;
                }
            }
            // Submit a pending query
            if (chunk->m_genQueryData.pending.size()) {
                // Submit for generation
                ChunkQuery* q = chunk->m_genQueryData.pending.back();
                chunk->m_genQueryData.current = q;
                m_threadPool->addTask(&q->genTask);
                chunk->m_genQueryData.pending.pop_back();
            }
        }
    }
}