#include "stdafx.h"
#include "VoxelMatrix.h"

#include <Vorb/colors.h>

ColorRGBA8 VoxelMatrix::getColor(const i32v3& position) const {
    if(position.x < 0 || position.x >= size.x) return color::Transparent;
    if(position.y < 0 || position.y >= size.y) return color::Transparent;
    if(position.z < 0 || position.z >= size.z) return color::Transparent;
    return data[position.x + position.y * size.x + position.z * size.x * size.y];
}

ColorRGBA8 VoxelMatrix::getColor(const i32 x, const i32 y, const i32 z) const {
    if(x < 0 || x >= size.x) return color::Transparent;
    if(y < 0 || y >= size.y) return color::Transparent;
    if(z < 0 || z >= size.z) return color::Transparent;
    return data[x + y * size.x + z * size.x * size.y];
}