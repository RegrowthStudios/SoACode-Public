///
/// VoxelCoordinateSpaces.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 27 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Contains the voxel coordinate space definitions
///

#pragma once

#ifndef VoxelCoordinateSpaces_h__
#define VoxelCoordinateSpaces_h__

enum WorldCubeFace {
    FACE_TOP = 0, FACE_LEFT, FACE_RIGHT,
    FACE_FRONT, FACE_BACK, FACE_BOTTOM
};

struct VoxelPosition {
    f64v3 position = f64v3(0.0);
    WorldCubeFace face = FACE_TOP;
    int rotation = 0;
};

struct ChunkGridPosition2D {
    i32v2 position = i32v2(0);
    WorldCubeFace face = FACE_TOP;
    int rotation = 0;
};

struct ChunkGridPosition3D {
    i32v3 position = i32v3(0);
    WorldCubeFace face = FACE_TOP;
    int rotation = 0;
};

struct VoxelGridPosition2D {
    i32v2 position = i32v2(0);
    WorldCubeFace face = FACE_TOP;
    int rotation = 0;
};

struct VoxelGridPosition3D {
    i32v3 position = i32v3(0);
    WorldCubeFace face = FACE_TOP;
    int rotation = 0;
};

struct ChunkFacePosition2D {
    i32v2 position = i32v2(0);
    WorldCubeFace face = FACE_TOP;
};

struct ChunkFacePosition3D {
    i32v2 position = i32v2(0);
    WorldCubeFace face = FACE_TOP;
};

struct VoxelFacePosition2D {
    i32v2 position = i32v2(0);
    WorldCubeFace face = FACE_TOP;
};

struct VoxelFacePosition3D {
    i32v2 position = i32v2(0);
    WorldCubeFace face = FACE_TOP;
};

#endif // VoxelCoordinateSpaces_h__
