#include "stdafx.h"
#include "ChunkGenerator.h"

#include "ChunkAllocator.h"
#include "Chunk.h"
#include "ChunkHandle.h"
#include "ChunkGrid.h"

void ChunkGenerator::init(vcore::ThreadPool<WorkerData>* threadPool,
                          PlanetGenData* genData,
                          ChunkGrid* grid) {
    m_threadPool = threadPool;
    m_proceduralGenerator.init(genData);
    m_grid = grid;
}

void ChunkGenerator::submitQuery(ChunkQuery* query) {
    Chunk& chunk = query->chunk;
    // Check if its already done
    if (chunk.genLevel >= query->genLevel) {
        query->m_isFinished = true;
        query->m_cond.notify_one();
        query->chunk.release();
        if (query->shouldRelease) query->release();
        return;
    }

    if (chunk.pendingGenLevel < query->genLevel) {
        chunk.pendingGenLevel = query->genLevel;
    }
    
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

void ChunkGenerator::finishQuery(ChunkQuery* query) {
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
                if (q2->shouldRelease) q2->release();
            }
            std::vector<ChunkQuery*>().swap(chunk.m_genQueryData.pending);
            // Notify listeners that this chunk is finished
            onGenFinish(q->chunk, q->genLevel);
            // Check for neighbor connect
            tryFlagMeshableNeighbors(q->chunk);
            q->chunk.release();
            if (q->shouldRelease) q->release();
        } else {
            // Otherwise possibly only some queries are done
            for (size_t i = 0; i < chunk.m_genQueryData.pending.size();) {
                auto q2 = chunk.m_genQueryData.pending[i];
                if (q2->genLevel <= chunk.genLevel) {
                    // TODO(Ben): This looks wrong. We don't remove from pending.
                    q2->m_isFinished = true;
                    q2->m_cond.notify_one();
                    chunk.isAccessible = true;
                    // TODO(Ben): Do we care about order?
                    chunk.m_genQueryData.pending[i] = chunk.m_genQueryData.pending.back();
                    chunk.m_genQueryData.pending.pop_back();
                    q2->chunk.release();
                    if (q2->shouldRelease) q2->release();
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
            // Notify listeners that this chunk is finished
            onGenFinish(q->chunk, q->genLevel);
            // Check for neighbor connect
            tryFlagMeshableNeighbors(q->chunk);
            q->chunk.release();
            if (q->shouldRelease) q->release();
        }
    }
}

#define FULL_26_BITS 0x3FFFFFF

void ChunkGenerator::tryFlagMeshableNeighbors(ChunkHandle& ch) {
    if (ch->genLevel != GEN_DONE || !ch->left.isAquired()) return;

    flagMeshbleNeighbor(ch->left, BIT(0));
    flagMeshbleNeighbor(ch->left->back, BIT(1));
    flagMeshbleNeighbor(ch->left->front, BIT(2));
    flagMeshbleNeighbor(ch->left->bottom, BIT(3));
    flagMeshbleNeighbor(ch->left->bottom->back, BIT(4));
    flagMeshbleNeighbor(ch->left->bottom->front, BIT(5));
    flagMeshbleNeighbor(ch->left->top, BIT(6));
    flagMeshbleNeighbor(ch->left->top->back, BIT(7));
    flagMeshbleNeighbor(ch->left->top->front, BIT(8));
    flagMeshbleNeighbor(ch->right, BIT(9));
    flagMeshbleNeighbor(ch->right->back, BIT(10));
    flagMeshbleNeighbor(ch->right->front, BIT(11));
    flagMeshbleNeighbor(ch->right->bottom, BIT(12));
    flagMeshbleNeighbor(ch->right->bottom->back, BIT(13));
    flagMeshbleNeighbor(ch->right->bottom->front, BIT(14));
    flagMeshbleNeighbor(ch->right->top, BIT(15));
    flagMeshbleNeighbor(ch->right->top->back, BIT(16));
    flagMeshbleNeighbor(ch->right->top->front, BIT(17));
    flagMeshbleNeighbor(ch->bottom, BIT(18));
    flagMeshbleNeighbor(ch->bottom->back, BIT(19));
    flagMeshbleNeighbor(ch->bottom->front, BIT(20));
    flagMeshbleNeighbor(ch->top, BIT(21));
    flagMeshbleNeighbor(ch->top->back, BIT(22));
    flagMeshbleNeighbor(ch->top->front, BIT(23));
    flagMeshbleNeighbor(ch->back, BIT(24));
    flagMeshbleNeighbor(ch->front, BIT(25));
}

inline void ChunkGenerator::flagMeshbleNeighbor(ChunkHandle& n, ui32 bit) {
    if (n->meshableNeighbors != FULL_26_BITS) {
        n->meshableNeighbors |= bit;
        if (n->meshableNeighbors == FULL_26_BITS) {
            m_grid->onNeighborsMeshable(n);
        }
    }
}

