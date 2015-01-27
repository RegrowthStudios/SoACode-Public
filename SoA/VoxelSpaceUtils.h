///
/// VoxelSpaceUtils.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 27 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Utilities for voxel coordinate space operations
///

#pragma once

#ifndef VoxelSpaceUtils_h__
#define VoxelSpaceUtils_h__

#include "VoxelCoordinateSpaces.h"

namespace VoxelSpaceUtils {
    /// Calculates the quaternion that can convert grid orientation to world orientation
    /// @param gridPosition: Voxel grid position
    /// @param worldRadius: Radius of the world in voxels
    /// @return quaternion that describes the relative rotation
    extern f64q calculateVoxelToSpaceQuat(const VoxelGridPosition2D& gridPosition, f64 worldRadius);

    /// Offsets a chunk grid position and handles rotation
    /// @param gridPosition: The chunk grid position
    /// @param xzOffset: The offset to apply, in chunks
    /// @param maxPos: Maximum grid position, for + and - direction
    extern void offsetChunkGridPosition(ChunkGridPosition2D& gridPosition, const i32v2& xzOffset, int maxPos);
}

#endif // VoxelSpaceUtils_h__