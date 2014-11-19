#include "stdafx.h"
#include "FloraTask.h"

#include "Chunk.h"
#include "FloraGenerator.h"
#include "ThreadPool.h"

void FloraTask::execute(vcore::WorkerData* workerData) {

    isSuccessful = false;
    if (FloraGenerator::generateFlora(chunk)) {
        isSuccessful = true;
    }
}
