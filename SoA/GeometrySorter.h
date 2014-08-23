#pragma once
#include <vector>

struct ChunkMesh;

struct DistanceStruct {
    i32 quadIndex;
    i32 distance;
};

class GeometrySorter {
public:
    static void sortTransparentBlocks(ChunkMesh* cm, const i32v3& cameraPos);

private:


    static std::vector <DistanceStruct> _distBuffer;
};