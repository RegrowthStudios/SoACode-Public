#include "stdafx.h"
#include "VoxPool.h"

#include "CAEngine.h"
#include "ChunkMesher.h"
#include "FloraGenerator.h"
#include "VoxelLightEngine.h"

WorkerData::~WorkerData() {
    delete chunkMesher;
    delete floraGenerator;
    delete voxelLightEngine;
    delete caEngine;
}
