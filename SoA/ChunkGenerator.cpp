#include "stdafx.h"
#include "ChunkGenerator.h"

#include "ChunkAllocator.h"
#include "Chunk.h"
#include "ChunkHandle.h"

void ChunkGenerator::init(vcore::ThreadPool<WorkerData>* threadPool,
                          PlanetGenData* genData) {
    m_threadPool = threadPool;
    m_proceduralGenerator.init(genData);
}

void ChunkGenerator::submitQuery(ChunkQuery* query) {
    Chunk& chunk = query->chunk;
    if (!chunk.gridData->isLoaded) {
        // If this heightmap isn't already loading, send it
        if (!chunk.gridData->isLoading) {
            // Send heightmap gen query
            chunk.gridData->isLoading = true;
            m_threadPool->addTask(&query->genTask);
        }
        // Store as a pending query
        m_pendingQueries[chunk.gridData].push_back(query);
    } else {
        if (chunk.m_genQueryData.current) {
            // Only one gen query should be active at a time so just store this one
            chunk.m_genQueryData.pending.push_back(query);
        } else {
            // Submit for generation
            chunk.m_genQueryData.current = query;
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
        Chunk& chunk = q->chunk;
        chunk.m_genQueryData.current = nullptr;
        // Check if it was a heightmap gen
        if (chunk.gridData->isLoading) {
            chunk.gridData->isLoaded = true;
            chunk.gridData->isLoading = false;

            // Submit all the pending queries on this grid data
            auto& it = m_pendingQueries.find(chunk.gridData); // TODO(Ben): Should this be shared? ( I don't think it should )
            for (auto& p : it->second) {
                submitQuery(p);
            }
            m_pendingQueries.erase(it);
        } else if (chunk.genLevel == GEN_DONE) {
            // If the chunk is done generating, we can signal all queries as done.
            for (auto& q2 : chunk.m_genQueryData.pending) {
                q2->m_isFinished = true;
                q2->m_cond.notify_one();
                chunk.isAccessible = true;
                q2->chunk.release();
                if (q2->shouldDelete) delete q2;
            }
            std::vector<ChunkQuery*>().swap(chunk.m_genQueryData.pending);
            q->chunk.release();
            if (q->shouldDelete) delete q;
        } else {
            // Otherwise possibly only some queries are done
            for (size_t i = 0; i < chunk.m_genQueryData.pending.size();) {
                auto q2 = chunk.m_genQueryData.pending[i];
                if (q2->genLevel <= chunk.genLevel) {
                    q2->m_isFinished = true;
                    q2->m_cond.notify_one();
                    chunk.isAccessible = true;
                    if (q2->shouldDelete) delete q2;
                    // TODO(Ben): Do we care about order?
                    chunk.m_genQueryData.pending[i] = chunk.m_genQueryData.pending.back();
                    chunk.m_genQueryData.pending.pop_back();
                    q2->chunk.release();
                } else {
                    i++;
                }
            }
            // Submit a pending query
            if (chunk.m_genQueryData.pending.size()) {
                // Submit for generation
                q = chunk.m_genQueryData.pending.back();
                chunk.m_genQueryData.pending.pop_back();
                chunk.m_genQueryData.current = q;
                m_threadPool->addTask(&q->genTask);
            }
            q->chunk.release();
            if (q->shouldDelete) delete q;
        }
    }
}