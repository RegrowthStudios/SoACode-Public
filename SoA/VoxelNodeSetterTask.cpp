#include "stdafx.h"
#include "VoxelNodeSetterTask.h"

#include "ChunkHandle.h"
#include "Chunk.h"

void VoxelNodeSetterTask::execute(WorkerData* workerData VORB_UNUSED) {
    {
        std::lock_guard<std::mutex> l(h->dataMutex);
        for (auto& node : forcedNodes) {
            h->blocks.set(node.blockIndex, node.blockID);
        }
        for (auto& node : condNodes) {
            // TODO(Ben): Custom condition
            if (h->blocks.get(node.blockIndex) == 0) {
                h->blocks.set(node.blockIndex, node.blockID);
            }
        }
    }

    if (h->genLevel >= GEN_DONE) h->DataChange(h);

    h.release();
}

void VoxelNodeSetterTask::cleanup() {
    // TODO(Ben): Better memory management.
    delete this;
}
