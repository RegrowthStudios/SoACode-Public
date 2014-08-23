#include "stdafx.h"

#include "GeometrySorter.h"

#include "ChunkRenderer.h"
#include "utils.h"

std::vector <DistanceStruct> GeometrySorter::_distBuffer;

i32 convertData(DistanceStruct* data) {
    return data->distance;
}

inline i32 getSquaredDist(i32v3 vec) {
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

bool comparator(const DistanceStruct& i, const DistanceStruct& j) { 
    return (i.distance > j.distance); 
}

void GeometrySorter::sortTransparentBlocks(ChunkMesh* cm, const i32v3& cameraPos) {

    _distBuffer.resize(cm->transQuadPositions.size());
    int dist;

    for (int i = 0; i < cm->transQuadPositions.size(); i++) {
        _distBuffer[i].quadIndex = i; 
        //We multiply by 2 because we need twice the precision of integers per block
        //we subtract by 1 in order to ensure that the camera position is centered on a block
        _distBuffer[i].distance = getSquaredDist((cm->position - cameraPos) * 2 - i32v3(1) + i32v3(cm->transQuadPositions[i]));
    }

   // radixSort<DistanceStruct, 8>(&(_distBuffer[0]), _distBuffer.size(), convertData, 31);
    std::sort(_distBuffer.begin(), _distBuffer.end(), comparator);

    int startIndex;
    int j = 0;
    for (int i = 0; i < _distBuffer.size(); i++) {
        startIndex = _distBuffer[i].quadIndex * 4;
        cm->transQuadIndices[j] = startIndex;
        cm->transQuadIndices[j + 1] = startIndex + 1;
        cm->transQuadIndices[j + 2] = startIndex + 2;
        cm->transQuadIndices[j + 3] = startIndex + 2;
        cm->transQuadIndices[j + 4] = startIndex + 3;
        cm->transQuadIndices[j + 5] = startIndex;
        j += 6;
    }
}