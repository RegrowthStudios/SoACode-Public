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
#include <Vorb/utils.h>

/// Namespace for converting between the different voxel and chunk coordinate spaces
namespace VoxelSpaceConversions {

#define W_X 0
#define W_Y 1
#define W_Z 2
    /// Maps face-space coordinate axis to world-space coordinates.
    /// [face]
    const i32v3 VOXEL_TO_WORLD[6] = {
        i32v3(W_X, W_Y, W_Z), // TOP
        i32v3(W_Z, W_X, W_Y), // LEFT
        i32v3(W_Z, W_X, W_Y), // RIGHT
        i32v3(W_X, W_Z, W_Y), // FRONT
        i32v3(W_X, W_Z, W_Y), // BACK
        i32v3(W_X, W_Y, W_Z) }; // BOTTOM
#undef W_X
#undef W_Y
#undef W_Z

    /// Multiply by the face-space Y axis in order to get the correct direction
    /// for its corresponding world space axis
    /// [face]
    const int FACE_Y_MULTS[6] = { 1, -1, 1, 1, -1, -1 };
    /// Multiply by the voxel-space X,Z axis in order to get the correct direction
    /// for its corresponding world-space axis
    /// [face]
    const i32v2 FACE_TO_WORLD_MULTS[6] = {
        i32v2(1, 1), // TOP
        i32v2(1, -1), // LEFT
        i32v2(-1, -1), // RIGHT
        i32v2(1, -1), // FRONT
        i32v2(-1, -1), // BACK
        i32v2(1, -1) }; // BOTTOM


    /// Gets multipliers for converting face direction to world direction
    extern f32v3 getCoordinateMults(const ChunkPosition2D& facePosition);
    extern f32v3 getCoordinateMults(const ChunkPosition3D& facePosition);
    /// Gets coordinate mappings for converting face position to world position
    extern i32v3 getCoordinateMapping(const ChunkPosition2D& facePosition);
    extern i32v3 getCoordinateMapping(const ChunkPosition3D& facePosition);

    /// Converts from voxel-space to chunk-space
    /// Does not affect rotation or face
    /// @param voxelPosition: The voxel grid position
    /// @return the chunk grid position
    extern ChunkPosition2D voxelToChunk(const VoxelPosition2D& voxelPosition);
    extern ChunkPosition3D voxelToChunk(const VoxelPosition3D& voxelPosition);
    extern i32v3 voxelToChunk(const i32v3& voxelPosition);
    extern i32v3 voxelToChunk(const f64v3& voxelPosition);
    /// Converts from chunk-space to voxel-space
    /// Does not affect rotation or face
    /// @param gridPosition: The chunk grid position
    /// @return the voxel position
    extern VoxelPosition2D chunkToVoxel(const ChunkPosition2D& gridPosition);
    extern VoxelPosition3D chunkToVoxel(const ChunkPosition3D& gridPosition);
   
    /// Converts from face-space to world-space
    /// @param facePosition: The face position
    /// @param voxelWorldRadius: Radius of the world in units of voxels
    /// @return the world position
    extern f64v3 voxelToWorld(const VoxelPosition2D& facePosition, f64 voxelWorldRadius);
    extern f64v3 voxelToWorld(const VoxelPosition3D& facePosition, f64 voxelWorldRadius);
    /// Converts from face-space to world-space
    /// @param facePosition: The face position
    /// @param voxelWorldRadius: Radius of the world in units of voxels
    /// @return the world position
    extern f64v3 chunkToWorld(const ChunkPosition2D& facePosition, f64 voxelWorldRadius);
    extern f64v3 chunkToWorld(const ChunkPosition3D& facePosition, f64 voxelWorldRadius);
    /// Converts from face-space to normalized world-space
    /// @param facePosition: The face position
    /// @param voxelWorldRadius: Radius of the world in units of voxels
    /// @return the normalized world position
    extern f64v3 voxelToWorldNormalized(const VoxelPosition2D& facePosition, f64 voxelWorldRadius);
    extern f64v3 voxelToWorldNormalized(const VoxelPosition3D& facePosition, f64 voxelWorldRadius);
    /// Converts from face-space to normalized world-space
    /// @param facePosition: The face position
    /// @param voxelWorldRadius: Radius of the world in units of voxels
    /// @return the normalized world position
    extern f64v3 chunkToWorldNormalized(const ChunkPosition2D& facePosition, f64 voxelWorldRadius);
    extern f64v3 chunkToWorldNormalized(const ChunkPosition3D& facePosition, f64 voxelWorldRadius);
    /// Converts from world-space to unrotated voxel-space
    /// @param worldSpace: World position in units of voxels. @pre should rotated so that the planet has
    ///   a relative orientation of 0
    /// @return the voxel position
    extern VoxelPosition3D worldToVoxel(const f64v3& worldPosition, f64 voxelWorldRadius);
}

#endif // VoxelSpaceConversions_h__