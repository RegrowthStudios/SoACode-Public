#include "stdafx.h"
#include "FloraTask.h"

#include "Chunk.h"
#include "ThreadPool.h"

void FloraTask::execute(vcore::WorkerData* workerData) {

    // Lazily initialize flora generator
    if (workerData->floraGenerator == nullptr) {
        workerData->floraGenerator = new FloraGenerator();
    }

    isSuccessful = false;
    if (workerData->floraGenerator->generateFlora(chunk, wnodes, lnodes)) {
        isSuccessful = true;
    }
}
