#include "stdafx.h"
#include "Chunk.h"
#include "ChunkGenerator.h"
#include "GenerateTask.h"

void GenerateTask::execute(WorkerData* workerData) {
    ChunkGenerator::generateChunk(chunk, loadData);
    delete loadData;
}
