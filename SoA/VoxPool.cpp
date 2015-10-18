#include "stdafx.h"
#include "VoxPool.h"

#include "CAEngine.h"
#include "ChunkMesher.h"
#include "VoxelLightEngine.h"

WorkerData::~WorkerData() {
    delete chunkMesher;
    delete voxelLightEngine;
}
