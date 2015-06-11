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
    if (!chunk->gridData->isLoaded) {
        // If this heightmap isn't already loading, send it
        if (!chunk->gridData->isLoading) {
            // Send heightmap gen query
            chunk->gridData->isLoading = true;
            m_threadPool->addTask(&query->genTask);
        }
        // Store as a pending query
        m_pendingQueries[chunk->gridData].push_back(query);
    } else {
        if (chunk->m_genQueryData.current) {
            // Only one gen query should be active at a time so just store this one
            chunk->m_genQueryData.pending.push_back(query);
        } else {
            // Submit for generation
            chunk->m_genQueryData.current = query;
            m_threadPool->addTask(&query->genTask);
        }
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
        ChunkQuery* q = queries[i];
        NChunk* chunk = q->getChunk();
        chunk->m_genQueryData.current = nullptr;
        // Check if it was a heightmap gen
        if (chunk->gridData->isLoading) {
            chunk->gridData->isLoaded = true;
            chunk->gridData->isLoading = false;

            // Submit all the pending queries on this grid data
            auto& it = m_pendingQueries.find(chunk->gridData);
            for (auto& p : it->second) {
                submitQuery(p);
            }
            m_pendingQueries.erase(it);
        } else if (chunk->genLevel == GEN_DONE) {
            if (q->shouldDelete) delete q;
            // If the chunk is done generating, we can signal all queries as done.
            for (auto& q2 : chunk->m_genQueryData.pending) {
                q2->m_isFinished = true;
                q2->m_cond.notify_one();
                q2->m_chunk->refCount--;
                if (q2->shouldDelete) delete q;
            }
            std::vector<ChunkQuery*>().swap(chunk->m_genQueryData.pending);
            chunk->refCount--;
        } else {
            if (q->shouldDelete) delete q;
            // Otherwise possibly only some queries are done
            for (size_t i = 0; i < chunk->m_genQueryData.pending.size();) {
                q = chunk->m_genQueryData.pending[i];
                if (q->genLevel <= chunk->genLevel) {
                    q->m_isFinished = true;
                    q->m_cond.notify_one();
                    q->m_chunk->refCount--;
                    if (q->shouldDelete) delete q;
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
                q = chunk->m_genQueryData.pending.back();
                chunk->m_genQueryData.pending.pop_back();
                chunk->m_genQueryData.current = q;
                m_threadPool->addTask(&q->genTask);
            }
            chunk->refCount--;
        }
    }
}