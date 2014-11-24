#include "stdafx.h"
#include "CellularAutomataTask.h"


CellularAutomataTask::CellularAutomataTask(Chunk* chunk, ui32 flags) : 
    _chunk(chunk),
    _flags(flags) {
}


void CellularAutomataTask::execute(WorkerData* workerData) {
    throw std::logic_error("The method or operation is not implemented.");
}
