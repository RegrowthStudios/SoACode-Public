#include "stdafx.h"
#include "GenerateTask.h"

#include "NChunk.h"
#include "ChunkGenerator.h"

void GenerateTask::execute(WorkerData* workerData) {
    NChunk* chunk = query->getChunk();
   // while (true) {
        chunkGenerator->m_proceduralGenerator.generate(chunk, heightData);
   // }
    chunk->genLevel = ChunkGenLevel::GEN_DONE;
    query->m_isFinished = true;
    query->m_cond.notify_one();
    chunkGenerator->onQueryFinish(query);
}
