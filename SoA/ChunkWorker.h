#pragma once
class ChunkMesher;

// Worker Thread Context
struct WorkerData {
public:
    volatile bool waiting;
    volatile bool stop;

    // Each Thread Gets Its Own Mesher
    ChunkMesher* chunkMesher;
};

// Only Loads Chunks
void WorkerThreadOld(WorkerData* data);
