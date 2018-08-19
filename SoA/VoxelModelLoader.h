///
/// VoxelModelLoader.h
/// Seed of Andromeda
///
/// Created by Frank McCoy on 7 April 2015
/// Copyright 2014-2015 Regrowth Studios
/// MIT License
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

class VoxelMatrix;

class VoxelModelLoader {
public:
    static bool loadModel(const nString& filePath, VoxelMatrix& matrix);
private:
    VoxelModelLoader();
};

#endif // VoxelModelLoader_h__