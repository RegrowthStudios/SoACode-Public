#pragma once
#include "stdafx.h"

inline int getXFromBlockIndex(int blockIndex) {
    return blockIndex & 0x1f;
}

inline int getYFromBlockIndex(int blockIndex) {
    return blockIndex >> 10;
}

inline int getZFromBlockIndex(int blockIndex) {
    return (blockIndex >> 5) & 0x1f;
}

inline void getPosFromBlockIndex(int blockIndex, int& x, int& y, int& z) {
    x = blockIndex & 0x1f;
    y = blockIndex >> 10;
    z = (blockIndex >> 5) & 0x1f;
}

inline void getPosFromBlockIndex(int blockIndex, i32v3& pos) {
    pos.x = blockIndex & 0x1f;
    pos.y = blockIndex >> 10;
    pos.z = (blockIndex >> 5) & 0x1f;
}

inline i32v3 getPosFromBlockIndex(int blockIndex) {
    return i32v3(blockIndex & 0x1f, blockIndex >> 10, (blockIndex >> 5) & 0x1f);
}

inline int getBlockIndexFromPos(int x, int y, int z) {
    return x | (y << 10) | (z << 5);
}

inline int getBlockIndexFromPos(const i32v3& pos) {
    return pos.x | (pos.y << 10) | (pos.z << 5);
}