#include "stdafx.h"
#include "CellularAutomataTask.h"

#include "CAEngine.h"
#include "Chunk.h"
#include "RenderTask.h"
#include "VoxPool.h"

CellularAutomataTask::CellularAutomataTask(ChunkManager* chunkManager,
                                           PhysicsEngine* physicsEngine,
                                           Chunk* chunk, bool makeMesh) : 
    IThreadPoolTask(true, CA_TASK_ID),
    m_chunkManager(chunkManager),
    m_physicsEngine(physicsEngine),
    _chunk(chunk) {

    return;

    typesToUpdate.reserve(2);

    if (makeMesh) {
        chunk->queuedForMesh = true;
        renderTask = new RenderTask();
        renderTask->init(_chunk, RenderTaskType::DEFAULT);
    }
}

void CellularAutomataTask::execute(WorkerData* workerData) {
    if (workerData->caEngine == nullptr) {
        workerData->caEngine = new CAEngine(m_chunkManager, m_physicsEngine);
    }
    workerData->caEngine->setChunk(_chunk);
    for (auto& type : typesToUpdate) {
        switch (type->getCaAlg()) {
            case CA_ALGORITHM::LIQUID:
   //             workerData->caEngine->updateLiquidBlocks(type->getCaIndex());
                break;
            case CA_ALGORITHM::POWDER:
   //             workerData->caEngine->updatePowderBlocks(type->getCaIndex());
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
