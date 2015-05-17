#pragma once
#ifndef VoxelMatrix_h__
#define VoxelMatrix_h__

class VoxelMatrix {
public:
    nString name;
    ui32v3 size;
    i32v3 position;
    ColorRGBA8* data;

    VoxelMatrix():
        name(),
        size(),
        position(),
        data(nullptr) {
        // Empty
    }

    ~VoxelMatrix() {
        delete[] data;
    }

    ColorRGBA8 getColor(const i32v3& position) const;
    ColorRGBA8 getColor(const i32 x, const i32 y, const i32 z) const;

    inline ui32 getIndex(const i32v3& position) const {
        return position.x + position.y * size.x + position.z * size.x * size.y;
    }

    inline ui32 getIndex(const i32 x, const i32 y, const i32 z) const {
        return x + y * size.x + z * size.x * size.y;
    }
};

#endif //VoxelMatrix_h__