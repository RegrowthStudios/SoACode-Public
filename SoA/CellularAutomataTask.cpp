#include "stdafx.h"
#include "CellularAutomataTask.h"

#include "CAEngine.h"
#include "Chunk.h"
#include "RenderTask.h"
#include "ThreadPool.h"

CellularAutomataTask::CellularAutomataTask(Chunk* chunk, bool makeMesh, ui32 flags) : 
    IThreadPoolTask(true, CA_TASK_ID),
    _chunk(chunk),
    _flags(flags) {

    if (makeMesh) {
        chunk->queuedForMesh = true;
        renderTask = new RenderTask();
        if (_flags & CA_FLAG_POWDER) {
            renderTask->init(_chunk, RenderTaskType::DEFAULT);
        } else if (_flags & CA_FLAG_LIQUID) {
            renderTask->init(_chunk, RenderTaskType::DEFAULT);
        }
    }
}

void CellularAutomataTask::execute(vcore::WorkerData* workerData) {
    if (workerData->caEngine == nullptr) {
        workerData->caEngine = new CAEngine;
    }
    workerData->caEngine->setChunk(_chunk);
    if (_flags & CA_FLAG_POWDER) {
        workerData->caEngine->updatePowderBlocks(0);
    }
    if (_flags & CA_FLAG_LIQUID) {
        workerData->caEngine->updateLiquidBlocks(0);
    }
    if (renderTask) {
        renderTask->execute(workerData);
    }
   // updateSpawnerBlocks(frameCounter == 0);
}
