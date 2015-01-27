#include "stdafx.h"
#include "VoxelSpaceConversions.h"

/// Defines the effect that transitioning from face i to face j will have on
/// rotation. Simply add to rotation value modulo 4
/// Each rotation represents a clockwise turn.
/// [source][destination]
const int FACE_TRANSITIONS[6][6] = {
    { 0, -1, 1, 0, 2, 0 }, // TOP
    { 1, 0, 0, 0, 0, -1 }, // LEFT
    { -1, 0, 0, 0, 0, 1 }, // RIGHT
    { 0, 0, 0, 0, 0, 0 }, // FRONT
    { 2, 0, 0, 0, 0, 2 }, // BACK
    { 0, 1, -1, 0, 2, 0 } }; // BOTTOM

/// Neighbors, starting from +x and moving clockwise
/// [face][rotation]
const int FACE_NEIGHBORS[6][4] = {
    { FACE_RIGHT, FACE_FRONT, FACE_LEFT, FACE_BACK }, // TOP
    { FACE_FRONT, FACE_BOTTOM, FACE_BACK, FACE_TOP }, // LEFT
    { FACE_BACK, FACE_BOTTOM, FACE_FRONT, FACE_TOP }, // RIGHT
    { FACE_RIGHT, FACE_BOTTOM, FACE_LEFT, FACE_TOP }, // FRONT
    { FACE_LEFT, FACE_BOTTOM, FACE_RIGHT, FACE_TOP }, // BACK
    { FACE_RIGHT, FACE_BACK, FACE_LEFT, FACE_FRONT } }; // BOTTOM


#define W_X 0
#define W_Y 1
#define W_Z 2
/// Maps face-space coordinate axis to world-space coordinates.
/// [face]
const i32v3 GRID_TO_WORLD[6] = {
      i32v3(W_X, W_Y, W_Z), // TOP
      i32v3(W_Z, W_X, W_Y), // LEFT
      i32v3(W_Z, W_X, W_Y), // RIGHT
      i32v3(W_X, W_Z, W_Y), // FRONT
      i32v3(W_X, W_Z, W_Y), // BACK
      i32v3(W_X, W_Y, W_Z) }; // BOTTOM
#undef W_X
#undef W_Y
#undef W_Z

/// Multiply by the grid-space X,Z axis in order to get the correct direction
/// for its corresponding world-space axis
const i32v2 FACE_COORDINATE_MULTS[6][4] = {
    { i32v2(1, -1), i32v2(1, 1), i32v2(-1, 1), i32v2(-1, -1) }, // TOP
    { i32v2(1, 1), i32v2(-1, 1), i32v2(-1, -1), i32v2(1, -1) }, // LEFT
    { i32v2(-1, 1), i32v2(-1, -1), i32v2(1, -1), i32v2(1, 1) }, // RIGHT
    { i32v2(1, 1), i32v2(-1, 1), i32v2(-1, -1), i32v2(1, -1) }, // FRONT
    { i32v2(-1, 1), i32v2(-1, -1), i32v2(1, -1), i32v2(1, 1) }, // BACK
    { i32v2(1, 1), i32v2(-1, 1), i32v2(-1, -1), i32v2(1, -1) } }; // BOTTOM

/// Multiply by the face-space Y axis in order to get the correct direction
/// for its corresponding world space axis
/// [face]
const int FACE_Y_MULTS[6] = { 1, -1, 1, 1, -1, -1 };

i32v2 VoxelSpaceConversions::gridToFace(const i32v2& gridPosition, int face, int rotation) {
    const i32v2& mult = FACE_COORDINATE_MULTS[face][rotation];

    i32v2 facePos(gridPosition * mult);

    if (rotation % 2) { //when rotation%2 x and z must switch
        std::swap(facePos.x, facePos.y);
    }

    return facePos;
}
i32v3 VoxelSpaceConversions::gridToFace(const i32v3& gridPosition, int face, int rotation) {
    const i32v2& mult = FACE_COORDINATE_MULTS[face][rotation];

    i32v3 facePos(gridPosition.x * mult.x, gridPosition.y, gridPosition.z * mult.y);

    if (rotation % 2) { //when rotation%2 x and z must switch
        std::swap(facePos.x, facePos.z);
    }

    return facePos;
}

f64v3 VoxelSpaceConversions::faceToWorld(const i32v2& facePosition, int face, f64 voxelWorldRadius) {
    return faceToWorldNormalized(facePosition, face, voxelWorldRadius) * voxelWorldRadius;
}
f64v3 VoxelSpaceConversions::faceToWorld(const i32v3& facePosition, int face, f64 voxelWorldRadius) {
    return faceToWorldNormalized(facePosition, face, voxelWorldRadius) * (voxelWorldRadius + facePosition.y);
}

f64v3 VoxelSpaceConversions::faceToWorldNormalized(const i32v2& facePosition, int face, f64 voxelWorldRadius) {
    const i32v3& axisMapping = GRID_TO_WORLD[face];

    f64v3 worldPosition;
    worldPosition[axisMapping.x] = facePosition.x;
    worldPosition[axisMapping.y] = voxelWorldRadius * FACE_Y_MULTS[face];
    worldPosition[axisMapping.z] = facePosition.y;

    return glm::normalize(worldPosition);
}
f64v3 VoxelSpaceConversions::faceToWorldNormalized(const i32v3& facePosition, int face, f64 voxelWorldRadius) {
    const i32v3& axisMapping = GRID_TO_WORLD[face];

    f64v3 worldPosition;
    worldPosition[axisMapping.x] = facePosition.x;
    worldPosition[axisMapping.y] = voxelWorldRadius * FACE_Y_MULTS[face];
    worldPosition[axisMapping.z] = facePosition.z;

    return glm::normalize(worldPosition);
}

extern f64v3 VoxelSpaceConversions::worldToVoxel(const f64v3& worldPosition, f64 voxelWorldRadius) {
    f64v3 boxIntersect;
    // Get closest point to the cube
    boxIntersect.x = (worldPosition.x <= -voxelWorldRadius) ? -voxelWorldRadius :
        ((worldPosition.x > voxelWorldRadius) ? (voxelWorldRadius) : worldPosition.x);
    boxIntersect.y = (worldPosition.y <= -voxelWorldRadius) ? -voxelWorldRadius :
        ((worldPosition.y > voxelWorldRadius) ? (voxelWorldRadius) : worldPosition.y);
    boxIntersect.z = (worldPosition.z <= -voxelWorldRadius) ? -voxelWorldRadius :
        ((worldPosition.z > voxelWorldRadius) ? (voxelWorldRadius) : worldPosition.z);

    if (boxIntersect.x == -voxelWorldRadius) {

    }
}
