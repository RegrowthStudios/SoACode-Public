///
/// VoxelSpaceConversions.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 27 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Converts between the different voxel coordinate spaces
///

#pragma once

#ifndef VoxelSpaceConversions_h__
#define VoxelSpaceConversions_h__

#include "Constants.h"
#include "VoxelCoordinateSpaces.h"
#include <Vorb/FastConversion.inl>

/// Namespace for converting between the different coordinate spaces
namespace VoxelSpaceConversions {
    /// Converts from voxel-space to grid-space
    /// @param voxelPosition: The floating point voxel position
    /// @return the grid position (1 unit == 1 chunk)
    inline i32v2 voxelToGrid(const f64v2& voxelPosition) {
        return i32v2(fastFloor(voxelPosition.x / CHUNK_WIDTH),
                     fastFloor(voxelPosition.y / CHUNK_WIDTH));
    }
    inline i32v3 voxelToGrid(const f64v3& voxelPosition) {
        return i32v3(fastFloor(voxelPosition.x / CHUNK_WIDTH),
                     fastFloor(voxelPosition.y / CHUNK_WIDTH),
                     fastFloor(voxelPosition.z / CHUNK_WIDTH));
    }
    /// Converts from grid-space to voxel-space
    /// @param gridPosition: The grid position
    /// @return the voxel position
    inline f64v2 gridToVoxel(const i32v2& gridPosition) { return f64v2(gridPosition * CHUNK_WIDTH); }
    inline f64v3 gridToVoxel(const i32v3& gridPosition) { return f64v3(gridPosition * CHUNK_WIDTH); }
    /// Converts from grid-space to face-space
    /// @param gridPosition: The grid position
    /// @param face: The cube face
    /// @param rotation: The grid rotation
    /// @return the face position
    extern i32v2 gridToFace(const i32v2& gridPosition, int face, int rotation);
    extern i32v3 gridToFace(const i32v3& gridPosition, int face, int rotation);
    /// Converts from face-space to world-space
    /// @param facePosition: The face position
    /// @param face: The cube face
    /// @param voxelWorldRadius: Radius of the world in units of voxels
    /// @return the world position
    extern f64v3 faceToWorld(const i32v2& facePosition, int face, f64 voxelWorldRadius);
    extern f64v3 faceToWorld(const i32v3& facePosition, int face, f64 voxelWorldRadius);
    /// Converts from face-space to normalized world-space
    /// @param facePosition: The face position
    /// @param face: The cube face
    /// @param voxelWorldRadius: Radius of the world in units of voxels
    /// @return the normalized world position
    extern f64v3 faceToWorldNormalized(const i32v2& facePosition, int face, f64 voxelWorldRadius);
    extern f64v3 faceToWorldNormalized(const i32v3& facePosition, int face, f64 voxelWorldRadius);
    /// Converts from world-space to unrotated voxel-space
    /// @param worldSpace: World position. @pre should rotated so that the planet has
    ///   a relative orientation of 0
    /// @return the voxel position
    extern f64v3 worldToVoxel(const f64v3& worldPosition, f64 voxelWorldRadius);
    /// Converts from world-space to face-space. Equivalent to worldToGrid
    inline i32v3 worldToFace(const f64v3& worldPosition, f64 voxelWorldRadius) {
        return voxelToGrid(worldToVoxel(worldPosition, voxelWorldRadius));
    }
    /// Converts from world-space to grid-space. Equivalent to worldToFace
    inline i32v3 worldToGrid(const f64v3& worldPosition, f64 voxelWorldRadius) {
        return voxelToGrid(worldToVoxel(worldPosition, voxelWorldRadius));
    }
}

#endif // VoxelSpaceConversions_h__