///
/// VoxelModelLoader.h
/// Seed of Andromeda
///
/// Created by Frank McCoy on 7 April 2015
/// Copyright 2014-2015 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Class to handle loading of VoxelModels.
/// Currently supports the Qubicle binary format.
///

#pragma once

#ifndef VoxelModelLoader_h__
#define VoxelModelLoader_h__

#include <Vorb/colors.h>

#include <vector>

#define CODE_FLAG 2
#define NEXT_SLICE_FLAG 6

class VoxelMatrix {
public:
    VoxelMatrix():
        name(),
        size(),
        position(),
        data(nullptr) 
    {
        // Empty
    }

    ~VoxelMatrix() {
        delete[] data;
    }

    nString name;
    ui32v3 size;
    i32v3 position;
    ColorRGBA8* data;

    ColorRGBA8 getColor(const i32v3& position) {
        if(position.x < 0 || position.x >= size.x) return color::Transparent;
        if(position.y < 0 || position.y >= size.y) return color::Transparent;
        if(position.z < 0 || position.z >= size.z) return color::Transparent;
        return data[position.x + position.y * size.x + position.z * size.x * size.y];
    }

    inline ui32 getIndex(const i32v3& position) {
        return position.x + position.y * size.x + position.z * size.x * size.y;
    }
};

class VoxelModelLoader {
public:
    VoxelModelLoader();
    ~VoxelModelLoader();

    static std::vector<VoxelMatrix*> loadModel(const nString& filePath);
};

#endif // VoxelModelLoader_h__