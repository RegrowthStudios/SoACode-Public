#include "stdafx.h"
#include "GenerateTask.h"

#include "Chunk.h"
#include "ChunkGenerator.h"

void GenerateTask::execute(WorkerData* workerData) {
    ChunkGenerator::generateChunk(chunk, loadData);
    delete loadData;
}
