#include "stdafx.h"
#include "DualContouringMesher.h"
#include "VoxelMatrix.h"
#include "Density.h"

#include <Vorb/Timing.h>

#include "Octree.h"

void DualContouringMesher::genMatrixMesh(const VoxelMatrix& matrix, std::vector<VoxelModelVertex>& vertices, std::vector<ui32>& indices) {
    int octreeSize = vmath::max(vmath::max(matrix.size.x, matrix.size.y), matrix.size.z);

    gMatrix = &matrix;
    octreeSize = 128;
    std::cout << octreeSize << std::endl;
    const int MAX_THRESHOLDS = 5;
    const float THRESHOLDS[MAX_THRESHOLDS] = { -1.f, 0.1f, 1.f, 10.f, 50.f };
    int thresholdIndex = 3;
    PreciseTimer timer;
    OctreeNode* root = BuildOctree(i32v3(-octreeSize / 2), octreeSize, THRESHOLDS[thresholdIndex]);
    std::cout << timer.stop() << std::endl;
    timer.start();
    GenerateMeshFromOctree(root, vertices, indices);
    std::cout << timer.stop() << std::endl;
}