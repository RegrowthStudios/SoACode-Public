#pragma once
class ChunkMesher;

// Worker Thread Context
struct WorkerData {
public:
	bool waiting;
	bool finished;
	bool stop;

	// Each Thread Gets Its Own Mesher
	ChunkMesher* chunkMesher;
};

// Only Loads Chunks
void WorkerThread(WorkerData* data);
