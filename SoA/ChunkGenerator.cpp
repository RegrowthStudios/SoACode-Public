#include "stdafx.h"
#include "ChunkGenerator.h"

#include "ChunkAllocator.h"

void ChunkGenerator::init(ChunkAllocator* chunkAllocator,
                          vcore::ThreadPool<WorkerData>* threadPool,
                          PlanetGenData* genData) {
    m_allocator = chunkAllocator;
    m_threadPool = threadPool;
    m_proceduralGenerator.init(genData);
}

void ChunkGenerator::submitQuery(ChunkQuery* query) {

}

void ChunkGenerator::onQueryFinish(ChunkQuery* query) {
    m_finishedQueries.enqueue(query);
}

// Updates finished queries
void ChunkGenerator::update() {

}