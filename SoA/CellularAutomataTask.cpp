#include "stdafx.h"
#include "CellularAutomataTask.h"

#include "CAEngine.h"
#include "ThreadPool.h"


CellularAutomataTask::CellularAutomataTask(Chunk* chunk, ui32 flags) : 
    _chunk(chunk),
    _flags(flags) {
}


void CellularAutomataTask::execute(vcore::WorkerData* workerData) {
    if (workerData->caEngine == nullptr) {
        workerData->caEngine = new CAEngine;
    }
    workerData->caEngine->setChunk(_chunk);
    if (_flags & CA_FLAG_POWDER) {
        workerData->caEngine->updatePowderBlocks();
    }
    if (_flags & CA_FLAG_LIQUID) {
        workerData->caEngine->updateLiquidBlocks();
    }

   // updateSpawnerBlocks(frameCounter == 0);
}
