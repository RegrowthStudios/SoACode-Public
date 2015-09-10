#pragma once

template <typename T>
inline T getXFromBlockIndex(T blockIndex) {
    return blockIndex & 0x1f;
}

template <typename T>
inline T getYFromBlockIndex(T blockIndex) {
    return blockIndex >> 10;
}

template <typename T>
inline T getZFromBlockIndex(T blockIndex) {
    return (blockIndex >> 5) & 0x1f;
}

template <typename T>
inline void getPosFromBlockIndex(T blockIndex, T& x, T& y, T& z) {
    x = blockIndex & 0x1f;
    y = blockIndex >> 10;
    z = (blockIndex >> 5) & 0x1f;
}

template <typename T>
inline void getPosFromBlockIndex(T blockIndex, vorb::Vector3<T>& pos) {
    pos.x = blockIndex & 0x1f;
    pos.y = blockIndex >> 10;
    pos.z = (blockIndex >> 5) & 0x1f;
}

template <typename T>
inline vorb::Vector3<T> getPosFromBlockIndex(T blockIndex) {
    return vorb::Vector3<T>(blockIndex & 0x1f, blockIndex >> 10, (blockIndex >> 5) & 0x1f);
}
static_assert(CHUNK_WIDTH == 32, "getPosFromBlockIndex assumes 32 chunk width");

template <typename T>
inline int getBlockIndexFromPos(T x, T y, T z) {
    return x | (y << 10) | (z << 5);
}

template <typename T>
inline int getBlockIndexFromPos(const T& pos) {
    return pos.x | (pos.y << 10) | (pos.z << 5);
}