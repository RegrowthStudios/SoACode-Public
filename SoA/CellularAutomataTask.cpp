#include "stdafx.h"
#include "CellularAutomataTask.h"

#include "CAEngine.h"
#include "Chunk.h"
#include "RenderTask.h"
#include "ThreadPool.h"

CellularAutomataTask::CellularAutomataTask(Chunk* chunk, bool makeMesh) : 
    IThreadPoolTask(true, CA_TASK_ID),
    _chunk(chunk) {

    typesToUpdate.reserve(2);

    if (makeMesh) {
        chunk->queuedForMesh = true;
        renderTask = new RenderTask();
        renderTask->init(_chunk, RenderTaskType::DEFAULT);
    }
}

void CellularAutomataTask::execute(vcore::WorkerData* workerData) {
    if (workerData->caEngine == nullptr) {
        workerData->caEngine = new CAEngine;
    }
    workerData->caEngine->setChunk(_chunk);
    for (auto& type : typesToUpdate) {
        switch (type->getCaAlg()) {
            case CA_ALGORITHM::LIQUID:
                workerData->caEngine->updateLiquidBlocks(type->getCaIndex());
                break;
            case CA_ALGORITHM::POWDER:
                workerData->caEngine->updatePowderBlocks(type->getCaIndex());
                break;
            default:
                break;
        }
    }

    if (renderTask) {
        renderTask->execute(workerData);
    }
   // updateSpawnerBlocks(frameCounter == 0);
}
