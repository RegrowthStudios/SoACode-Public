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
    FACE_FRONT, FACE_BACK, FACE_BOTTOM,
    FACE_NONE
};

struct ChunkPosition2D {
    i32v2 pos = i32v2(0);
    WorldCubeFace face = FACE_TOP;
};

struct ChunkPosition3D {
    i32v3 pos = i32v3(0);
    WorldCubeFace face = FACE_TOP;
};

struct VoxelPosition2D {
    f64v2 pos = f64v2(0.0);
    WorldCubeFace face = FACE_TOP;
};

struct VoxelPosition3D {
    f64v3 pos = f64v3(0.0);
    WorldCubeFace face = FACE_TOP;
};

#endif // VoxelCoordinateSpaces_h__
