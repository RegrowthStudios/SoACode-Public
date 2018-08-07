#include "stdafx.h"
#include "VoxelMatrix.h"

#include <Vorb/colors.h>

const ColorRGBA8& VoxelMatrix::getColor(const int index) const {
    return data[index];
}

const ColorRGBA8& VoxelMatrix::getColor(const i32v3& position) const {
    return data[position.x + position.y * size.x + position.z * size.x * size.y];
}

const ColorRGBA8& VoxelMatrix::getColor(const i32 x, const i32 y, const i32 z) const {
    return data[x + y * size.x + z * size.x * size.y];
}

const ColorRGBA8& VoxelMatrix::getColorAndCheckBounds(const i32v3& position) const {
    if (position.x < 0 || position.x >= (i32)size.x) return color::Transparent;
    if (position.y < 0 || position.y >= (i32)size.y) return color::Transparent;
    if (position.z < 0 || position.z >= (i32)size.z) return color::Transparent;
    return data[position.x + position.y * size.x + position.z * size.x * size.y];
}

const ColorRGBA8& VoxelMatrix::getColorAndCheckBounds(const i32 x, const i32 y, const i32 z) const {
    if (x < 0 || x >= (i32)size.x) return color::Transparent;
    if (y < 0 || y >= (i32)size.y) return color::Transparent;
    if (z < 0 || z >= (i32)size.z) return color::Transparent;
    return data[x + y * size.x + z * size.x * size.y];
}

bool VoxelMatrix::isInterior(const i32v3& position) const {
    if (getColorAndCheckBounds(position + i32v3(1, 0, 0)).a == 0) return false;
    if (getColorAndCheckBounds(position + i32v3(0, 1, 0)).a == 0) return false;
    if (getColorAndCheckBounds(position + i32v3(0, 0, 1)).a == 0) return false;
    if (getColorAndCheckBounds(position + i32v3(-1, 0, 0)).a == 0) return false;
    if (getColorAndCheckBounds(position + i32v3(0, -1, 0)).a == 0) return false;
    if (getColorAndCheckBounds(position + i32v3(0, 0, -1)).a == 0) return false;
    return true;
}

bool VoxelMatrix::isInterior(const i32 x, const i32 y, const i32 z) const {
    return isInterior(i32v3(x, y, z));
}