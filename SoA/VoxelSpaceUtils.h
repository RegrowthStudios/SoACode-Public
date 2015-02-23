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
    extern f64q calculateVoxelToSpaceQuat(const VoxelPosition2D& gridPosition, f64 worldRadius);
    extern f64q calculateVoxelToSpaceQuat(const VoxelPosition3D& gridPosition, f64 worldRadius);

    /// Offsets a chunk grid position and handles rotation
    /// @param gridPosition: The chunk grid position
    /// @param xzOffset: The offset to apply, in chunks
    /// @param maxPos: Maximum grid position, for + and - direction
    extern void offsetChunkGridPosition(ChunkPosition2D& gridPosition, const i32v2& xzOffset, int maxPos);

    // TODO(Ben): implement this with new stuff
    /* void getIterationConstants(OUT int& jStart, OUT int& jMult, OUT int& jEnd, OUT int& jInc, OUT int& kStart, OUT int& kMult, OUT int& kEnd, OUT int& kInc) {
        switch (rotation) { //we use rotation value to un-rotate the chunk data
            case 0: //no rotation
                jStart = 0;
                kStart = 0;
                jEnd = kEnd = CHUNK_WIDTH;
                jInc = kInc = 1;
                jMult = CHUNK_WIDTH;
                kMult = 1;
                break;
            case 1: //up is right
                jMult = 1;
                jStart = CHUNK_WIDTH - 1;
                jEnd = -1;
                jInc = -1;
                kStart = 0;
                kEnd = CHUNK_WIDTH;
                kInc = 1;
                kMult = CHUNK_WIDTH;
                break;
            case 2: //up is down
                jMult = CHUNK_WIDTH;
                jStart = CHUNK_WIDTH - 1;
                kStart = CHUNK_WIDTH - 1;
                jEnd = kEnd = -1;
                jInc = kInc = -1;
                kMult = 1;
                break;
            case 3: //up is left
                jMult = 1;
                jStart = 0;
                jEnd = CHUNK_WIDTH;
                jInc = 1;
                kMult = CHUNK_WIDTH;
                kStart = CHUNK_WIDTH - 1;
                kEnd = -1;
                kInc = -1;
                break;
        }
    } */
}

#endif // VoxelSpaceUtils_h__