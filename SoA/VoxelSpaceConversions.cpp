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

ChunkGridPosition2D VoxelSpaceConversions::voxelGridToChunkGrid(const VoxelGridPosition2D& voxelPosition) {
    ChunkGridPosition2D gpos;
    gpos.face = voxelPosition.face;
    gpos.rotation = voxelPosition.rotation;
    gpos.pos.x = fastFloor(voxelPosition.pos.x / CHUNK_WIDTH);
    gpos.pos.y = fastFloor(voxelPosition.pos.y / CHUNK_WIDTH);
    return gpos;
}
ChunkGridPosition3D VoxelSpaceConversions::voxelGridToChunkGrid(const VoxelGridPosition3D& voxelPosition) {
    ChunkGridPosition3D gpos;
    gpos.face = voxelPosition.face;
    gpos.rotation = voxelPosition.rotation;
    gpos.pos.x = fastFloor(voxelPosition.pos.x / CHUNK_WIDTH);
    gpos.pos.y = fastFloor(voxelPosition.pos.y / CHUNK_WIDTH);
    gpos.pos.z = fastFloor(voxelPosition.pos.z / CHUNK_WIDTH);
    return gpos;
}

VoxelGridPosition2D VoxelSpaceConversions::chunkGridToVoxelGrid(const ChunkGridPosition2D& gridPosition) {
    VoxelGridPosition2D vpos;
    vpos.face = gridPosition.face;
    vpos.rotation = gridPosition.rotation;
    vpos.pos.x = gridPosition.pos.x * CHUNK_WIDTH;
    vpos.pos.y = gridPosition.pos.y * CHUNK_WIDTH;
    return vpos;
}
VoxelGridPosition3D VoxelSpaceConversions::chunkGridToVoxelGrid(const ChunkGridPosition3D& gridPosition) {
    VoxelGridPosition3D vpos;
    vpos.face = gridPosition.face;
    vpos.rotation = gridPosition.rotation;
    vpos.pos.x = gridPosition.pos.x * CHUNK_WIDTH;
    vpos.pos.y = gridPosition.pos.y * CHUNK_WIDTH;
    vpos.pos.z = gridPosition.pos.z * CHUNK_WIDTH;
    return vpos;
}

ChunkFacePosition2D VoxelSpaceConversions::voxelFaceToChunkFace(const VoxelFacePosition2D& voxelPosition) {
    ChunkFacePosition2D gpos;
    gpos.face = voxelPosition.face;
    gpos.pos.x = fastFloor(voxelPosition.pos.x / CHUNK_WIDTH);
    gpos.pos.y = fastFloor(voxelPosition.pos.y / CHUNK_WIDTH);
    return gpos;
}

ChunkFacePosition3D VoxelSpaceConversions::voxelFaceToChunkFace(const VoxelFacePosition3D& voxelPosition) {
    ChunkFacePosition3D gpos;
    gpos.face = voxelPosition.face;
    gpos.pos.x = fastFloor(voxelPosition.pos.x / CHUNK_WIDTH);
    gpos.pos.y = fastFloor(voxelPosition.pos.y / CHUNK_WIDTH);
    gpos.pos.z = fastFloor(voxelPosition.pos.z / CHUNK_WIDTH);
    return gpos;
}

VoxelFacePosition2D VoxelSpaceConversions::chunkFaceToVoxelFace(const ChunkFacePosition2D& gridPosition) {
    VoxelFacePosition2D vpos;
    vpos.face = gridPosition.face;
    vpos.pos.x = gridPosition.pos.x * CHUNK_WIDTH;
    vpos.pos.y = gridPosition.pos.y * CHUNK_WIDTH;
    return vpos;
}

VoxelFacePosition3D VoxelSpaceConversions::chunkFaceToVoxelFace(const ChunkFacePosition3D& gridPosition) {
    VoxelFacePosition3D vpos;
    vpos.face = gridPosition.face;
    vpos.pos.x = gridPosition.pos.x * CHUNK_WIDTH;
    vpos.pos.y = gridPosition.pos.y * CHUNK_WIDTH;
    vpos.pos.z = gridPosition.pos.z * CHUNK_WIDTH;
    return vpos;
}

VoxelFacePosition2D VoxelSpaceConversions::voxelGridToFace(const VoxelGridPosition2D& gridPosition) {
    const i32v2& mult = FACE_COORDINATE_MULTS[gridPosition.face][gridPosition.rotation];

    VoxelFacePosition2D facePos;
    facePos.face = gridPosition.face;
    facePos.pos = gridPosition.pos * f64v2(mult);

    if (gridPosition.rotation % 2) { //when rotation%2 x and z must switch
        std::swap(facePos.pos.x, facePos.pos.y);
    }

    return facePos;
}
VoxelFacePosition3D VoxelSpaceConversions::voxelGridToFace(const VoxelGridPosition3D& gridPosition) {
    const i32v2& mult = FACE_COORDINATE_MULTS[gridPosition.face][gridPosition.rotation];

    VoxelFacePosition3D facePos;
    facePos.face = gridPosition.face;
    facePos.pos = f64v3(gridPosition.pos.x * mult.x, gridPosition.pos.y, gridPosition.pos.z * mult.y);

    if (gridPosition.rotation % 2) { //when rotation%2 x and z must switch
        std::swap(facePos.pos.x, facePos.pos.z);
    }

    return facePos;
}

ChunkFacePosition2D VoxelSpaceConversions::chunkGridToFace(const ChunkGridPosition2D& gridPosition) {
    const i32v2& mult = FACE_COORDINATE_MULTS[gridPosition.face][gridPosition.rotation];

    ChunkFacePosition2D facePos;
    facePos.face = gridPosition.face;
    facePos.pos = gridPosition.pos * mult;

    if (gridPosition.rotation % 2) { //when rotation%2 x and z must switch
        std::swap(facePos.pos.x, facePos.pos.y);
    }

    return facePos;
}
ChunkFacePosition3D VoxelSpaceConversions::chunkGridToFace(const ChunkGridPosition3D& gridPosition) {
    const i32v2& mult = FACE_COORDINATE_MULTS[gridPosition.face][gridPosition.rotation];

    ChunkFacePosition3D facePos;
    facePos.face = gridPosition.face;
    facePos.pos = i32v3(gridPosition.pos.x * mult.x, gridPosition.pos.y, gridPosition.pos.z * mult.y);

    if (gridPosition.rotation % 2) { //when rotation%2 x and z must switch
        std::swap(facePos.pos.x, facePos.pos.z);
    }

    return facePos;
}

f64v3 VoxelSpaceConversions::voxelFaceToWorld(const VoxelFacePosition2D& facePosition, f64 voxelWorldRadius) {
    return voxelFaceToWorldNormalized(facePosition, voxelWorldRadius) * voxelWorldRadius;
}
f64v3 VoxelSpaceConversions::voxelFaceToWorld(const VoxelFacePosition3D& facePosition, f64 voxelWorldRadius) {
    return voxelFaceToWorldNormalized(facePosition, voxelWorldRadius) * (voxelWorldRadius + facePosition.pos.y);
}

f64v3 VoxelSpaceConversions::chunkFaceToWorld(const ChunkFacePosition2D& facePosition, f64 voxelWorldRadius) {
    return chunkFaceToWorldNormalized(facePosition, voxelWorldRadius) * voxelWorldRadius;
}
f64v3 VoxelSpaceConversions::chunkFaceToWorld(const ChunkFacePosition3D& facePosition, f64 voxelWorldRadius) {
    return chunkFaceToWorldNormalized(facePosition, voxelWorldRadius) * (voxelWorldRadius + facePosition.pos.y * CHUNK_WIDTH);
}

f64v3 VoxelSpaceConversions::voxelFaceToWorldNormalized(const VoxelFacePosition2D& facePosition, f64 voxelWorldRadius) {
    const i32v3& axisMapping = GRID_TO_WORLD[facePosition.face];

    f64v3 worldPosition;
    worldPosition[axisMapping.x] = facePosition.pos.x;
    worldPosition[axisMapping.y] = voxelWorldRadius * FACE_Y_MULTS[facePosition.face];
    worldPosition[axisMapping.z] = facePosition.pos.y;

    return glm::normalize(worldPosition);
}
f64v3 VoxelSpaceConversions::voxelFaceToWorldNormalized(const VoxelFacePosition3D& facePosition, f64 voxelWorldRadius) {
    const i32v3& axisMapping = GRID_TO_WORLD[facePosition.face];

    f64v3 worldPosition;
    worldPosition[axisMapping.x] = facePosition.pos.x;
    worldPosition[axisMapping.y] = voxelWorldRadius * FACE_Y_MULTS[facePosition.face];
    worldPosition[axisMapping.z] = facePosition.pos.z;

    return glm::normalize(worldPosition);
}

f64v3 VoxelSpaceConversions::chunkFaceToWorldNormalized(const ChunkFacePosition2D& facePosition, f64 voxelWorldRadius) {
    return voxelFaceToWorldNormalized(chunkFaceToVoxelFace(facePosition), voxelWorldRadius);
}
f64v3 VoxelSpaceConversions::chunkFaceToWorldNormalized(const ChunkFacePosition3D& facePosition, f64 voxelWorldRadius) {
    return voxelFaceToWorldNormalized(chunkFaceToVoxelFace(facePosition), voxelWorldRadius);
}

VoxelGridPosition3D VoxelSpaceConversions::worldToVoxelGrid(const f64v3& worldPosition, f64 voxelWorldRadius) {
    f64v3 boxIntersect;
    VoxelGridPosition3D gpos;

    // Get closest point to the cube
    boxIntersect.x = (worldPosition.x <= -voxelWorldRadius) ? -voxelWorldRadius :
        ((worldPosition.x > voxelWorldRadius) ? (voxelWorldRadius) : worldPosition.x);
    boxIntersect.y = (worldPosition.y <= -voxelWorldRadius) ? -voxelWorldRadius :
        ((worldPosition.y > voxelWorldRadius) ? (voxelWorldRadius) : worldPosition.y);
    boxIntersect.z = (worldPosition.z <= -voxelWorldRadius) ? -voxelWorldRadius :
        ((worldPosition.z > voxelWorldRadius) ? (voxelWorldRadius) : worldPosition.z);

    f64 height = glm::length(worldPosition - glm::normalize(boxIntersect) * voxelWorldRadius);

    if (boxIntersect.x == -voxelWorldRadius) {
        gpos.face = FACE_LEFT;
        gpos.pos.x = boxIntersect.z;
        gpos.pos.y = height;
        gpos.pos.z = boxIntersect.y;
    } else if (boxIntersect.x == voxelWorldRadius) {
        gpos.face = FACE_RIGHT;
        gpos.pos.x = -boxIntersect.z;
        gpos.pos.y = height;
        gpos.pos.z = boxIntersect.y;
    } else if (boxIntersect.y == -voxelWorldRadius) {
        gpos.face = FACE_BOTTOM;
        gpos.pos.x = boxIntersect.x;
        gpos.pos.y = height;
        gpos.pos.z = boxIntersect.z;
    } else if (boxIntersect.y == voxelWorldRadius) {
        gpos.face = FACE_TOP;
        gpos.pos.x = boxIntersect.x;
        gpos.pos.y = height;
        gpos.pos.z = -boxIntersect.z;
    } else if (boxIntersect.z == -voxelWorldRadius) {
        gpos.face = FACE_BACK;
        gpos.pos.x = -boxIntersect.x;
        gpos.pos.y = height;
        gpos.pos.z = boxIntersect.y;
    } else if (boxIntersect.z == voxelWorldRadius) {
        gpos.face = FACE_FRONT;
        gpos.pos.x = boxIntersect.x;
        gpos.pos.y = height;
        gpos.pos.z = boxIntersect.y;
    }

    return gpos;
}
