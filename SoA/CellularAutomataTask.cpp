#include "stdafx.h"
#include "CellularAutomataTask.h"

#include "CAEngine.h"
#include "Chunk.h"
#include "ChunkMeshManager.h"
#include "VoxPool.h"

CellularAutomataTask::CellularAutomataTask(ChunkManager* chunkManager,
                                           PhysicsEngine* physicsEngine,
                                           Chunk* chunk,
                                           OPT ChunkMeshManager* meshManager VORB_MAYBE_UNUSED) :
                                           IThreadPoolTask(CA_TASK_ID),
                                           _chunk(chunk),
                                           m_chunkManager(chunkManager),
                                           m_physicsEngine(physicsEngine) {

    //return;

    //typesToUpdate.reserve(2);

    //if (meshManager) {
    //    chunk->queuedForMesh = true;
    //    renderTask = new RenderTask();
    //    renderTask->init(_chunk, RenderTaskType::DEFAULT, meshManager);
    //}
}

void CellularAutomataTask::execute(WorkerData* workerData VORB_UNUSED) {
   // if (workerData->caEngine == nullptr) {
   //     workerData->caEngine = new CAEngine(m_chunkManager, m_physicsEngine);
   // }
   // workerData->caEngine->setChunk(_chunk);
   // for (auto& type : typesToUpdate) {
   //     switch (type->getCaAlg()) {
   //         case CA_ALGORITHM::LIQUID:
   ////             workerData->caEngine->updateLiquidBlocks(type->getCaIndex());
   //             break;
   //         case CA_ALGORITHM::POWDER:
   ////             workerData->caEngine->updatePowderBlocks(type->getCaIndex());
   //             break;
   //         default:
   //             break;
   //     }
   // }

   // if (renderTask) {
   //     renderTask->execute(workerData);
   // }
   //// updateSpawnerBlocks(frameCounter == 0);
}
