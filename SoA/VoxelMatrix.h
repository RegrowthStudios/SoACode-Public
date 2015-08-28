#pragma once
#ifndef VoxelMatrix_h__
#define VoxelMatrix_h__

class VoxelMatrix {
public:
    const ColorRGBA8& getColor(const int index) const;

    const ColorRGBA8& getColor(const i32v3& position) const;
    const ColorRGBA8& getColor(const i32 x, const i32 y, const i32 z) const;

    const ColorRGBA8& getColorAndCheckBounds(const i32v3& position) const;
    const ColorRGBA8& getColorAndCheckBounds(const i32 x, const i32 y, const i32 z) const;

    inline ui32 getIndex(const i32v3& position) const {
       // return position.x + position.y * size.x + position.z * size.x * size.y;
        return 0;
    }

    inline ui32 getIndex(const i32 x, const i32 y, const i32 z) const {
       // return x + y * size.x + z * size.x * size.y;
        return 0;
    }

    bool isInterior(const i32v3& position) const;
    bool isInterior(const i32 x, const i32 y, const i32 z) const;

    void dispose() {
        delete[] data;
        data = nullptr;
    }

    nString name = "";
    ui32v3 size;
    i32v3 position;
    ColorRGBA8* data = nullptr;
};

#endif //VoxelMatrix_h__