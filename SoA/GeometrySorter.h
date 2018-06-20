#pragma once

#include <vector>
#include "Vorb/types.h"

class ChunkMesh;

class Distanceclass {
public:
    i32 quadIndex;
    i32 distance;
};

class GeometrySorter {
public:
    static void sortTransparentBlocks(ChunkMesh* cm, const i32v3& cameraPos);

private:


    static std::vector <Distanceclass> _distBuffer;
};