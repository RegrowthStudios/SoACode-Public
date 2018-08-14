#include "stdafx.h"
#include "VoxelSpaceUtils.h"
#include "VoxelSpaceConversions.h"


/// Defines the effect that transitioning from face i to face j will have on
/// rotation. Simply add to rotation value modulo 4
/// Each rotation represents a clockwise turn.
/// [source][destination]
// const int FACE_TRANSITIONS[6][6] = {
//     { 0, -1, 1, 0, 2, 0 }, // TOP
//     { 1, 0, 0, 0, 0, -1 }, // LEFT
//     { -1, 0, 0, 0, 0, 1 }, // RIGHT
//     { 0, 0, 0, 0, 0, 0 }, // FRONT
//     { 2, 0, 0, 0, 0, 2 }, // BACK
//     { 0, 1, -1, 0, 2, 0 } }; // BOTTOM

/// Neighbors, starting from +x and moving clockwise
/// [face][rotation]
// const WorldCubeFace FACE_NEIGHBORS[6][4] = {
//     { FACE_RIGHT, FACE_FRONT, FACE_LEFT, FACE_BACK }, // TOP
//     { FACE_FRONT, FACE_BOTTOM, FACE_BACK, FACE_TOP }, // LEFT
//     { FACE_BACK, FACE_BOTTOM, FACE_FRONT, FACE_TOP }, // RIGHT
//     { FACE_RIGHT, FACE_BOTTOM, FACE_LEFT, FACE_TOP }, // FRONT
//     { FACE_LEFT, FACE_BOTTOM, FACE_RIGHT, FACE_TOP }, // BACK
//     { FACE_RIGHT, FACE_BACK, FACE_LEFT, FACE_FRONT } }; // BOTTOM


//f64q q1 = quatBetweenVectors(FACE_NORMALS[0], VoxelSpaceConversions::voxelFaceToWorldNormalized(
//    VoxelSpaceConversions::voxelGridToFace(gridPosition), worldRadius));
//f64q q2 = quatBetweenVectors(f32v3(1.0, 0.0, 0.0), q1 * f32v3(1.0, 0.0, 0.0))

#define OFFSET 1.0

/// This is basically a hack with normal mapping tangent space stuff
f64q VoxelSpaceUtils::calculateVoxelToSpaceQuat(const VoxelPosition2D& gridPosition, f64 worldRadius) {


    VoxelPosition2D gp2 = gridPosition;
    gp2.pos.x += OFFSET;
    VoxelPosition2D gp3 = gridPosition;
    gp3.pos.y += OFFSET;

    f64v3 v1 = VoxelSpaceConversions::voxelToWorldNormalized(
        gridPosition, worldRadius);
    f64v3 v2 = VoxelSpaceConversions::voxelToWorldNormalized(
        gp2, worldRadius);
    f64v3 v3 = VoxelSpaceConversions::voxelToWorldNormalized(
        gp3, worldRadius);
  
    f64v3 tangent = glm::normalize(v2 - v1);
    f64v3 biTangent = glm::normalize(v3 - v1);

    f64m4 worldRotationMatrix;
    worldRotationMatrix[0] = f64v4(tangent, 0);
    worldRotationMatrix[1] = f64v4(v1, 0);
    worldRotationMatrix[2] = f64v4(biTangent, 0);
    worldRotationMatrix[3] = f64v4(0, 0, 0, 1);

    return glm::quat_cast(worldRotationMatrix);
}
f64q VoxelSpaceUtils::calculateVoxelToSpaceQuat(const VoxelPosition3D& gridPosition, f64 worldRadius) {

    VoxelPosition3D gp2 = gridPosition;
    gp2.pos.x += OFFSET;

    f64v3 v1 = VoxelSpaceConversions::voxelToWorldNormalized(
        gridPosition, worldRadius);
    f64v3 v2 = VoxelSpaceConversions::voxelToWorldNormalized(
        gp2, worldRadius);

    f64v3 tangent = glm::normalize(v2 - v1);
    f64v3 biTangent = glm::cross(tangent, v1);
   
    f64m4 worldRotationMatrix;
    worldRotationMatrix[0] = f64v4(tangent, 0);
    worldRotationMatrix[1] = f64v4(v1, 0);
    worldRotationMatrix[2] = f64v4(biTangent, 0);
    worldRotationMatrix[3] = f64v4(0, 0, 0, 1);

    return glm::quat_cast(worldRotationMatrix);
}

void VoxelSpaceUtils::offsetChunkGridPosition(OUT ChunkPosition2D& gridPosition, const i32v2& xzOffset, int maxPos VORB_UNUSED) {
    gridPosition.pos += xzOffset;
    WorldCubeFace newFace = gridPosition.face;

    //TODO(Ben): New transition logic and then remove VORB_UNUSED tags.
    //if (gridPosition.pos.y < -maxPos) { // BOTTOM SIDE
    //    gridPosition.pos.y += maxPos;

    //    newFace = FACE_NEIGHBORS[gridPosition.face][(1 + gridPosition.rotation) % 4];
    //    gridPosition.rotation += FACE_TRANSITIONS[gridPosition.face][newFace];
    //    if (gridPosition.rotation < 0) { gridPosition.rotation += 4; } else { gridPosition.rotation %= 4; }
    //} else if (gridPosition.pos.y > maxPos) { // TOP SIDE
    //    gridPosition.pos.y -= maxPos;

    //    newFace = FACE_NEIGHBORS[gridPosition.face][(3 + gridPosition.rotation) % 4];
    //    gridPosition.rotation += FACE_TRANSITIONS[gridPosition.face][newFace];
    //    if (gridPosition.rotation < 0) { gridPosition.rotation += 4; } else { gridPosition.rotation %= 4; }
    //}

    //if (gridPosition.pos.x < -maxPos) { // LEFT SIDE
    //    gridPosition.pos.x += maxPos;

    //    newFace = FACE_NEIGHBORS[gridPosition.face][(2 + gridPosition.rotation) % 4];
    //    gridPosition.rotation += FACE_TRANSITIONS[gridPosition.face][newFace];
    //    if (gridPosition.rotation < 0) { gridPosition.rotation += 4; } else { gridPosition.rotation %= 4; }
    //} else if (gridPosition.pos.x > maxPos) { // RIGHT SIDE
    //    gridPosition.pos.x -= maxPos;

    //    newFace = FACE_NEIGHBORS[gridPosition.face][gridPosition.rotation];
    //    gridPosition.rotation += FACE_TRANSITIONS[gridPosition.face][newFace];
    //    if (gridPosition.rotation < 0) { gridPosition.rotation += 4; } else { gridPosition.rotation %= 4; }
    //}

    gridPosition.face = newFace;
}
