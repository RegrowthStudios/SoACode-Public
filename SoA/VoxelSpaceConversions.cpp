#include "stdafx.h"
#include "VoxelSpaceConversions.h"
#include <Vorb/utils.h>

f32v3 VoxelSpaceConversions::getCoordinateMults(const ChunkPosition2D& facePosition) {
    f32v3 rv;
    rv.x = (float)FACE_TO_WORLD_MULTS[facePosition.face].x;
    rv.y = (float)FACE_Y_MULTS[facePosition.face];
    rv.z = (float)FACE_TO_WORLD_MULTS[facePosition.face].y;
    return rv;
}
f32v3 VoxelSpaceConversions::getCoordinateMults(const ChunkPosition3D& facePosition) {
    f32v3 rv;
    rv.x = (float)FACE_TO_WORLD_MULTS[facePosition.face].x;
    rv.y = (float)FACE_Y_MULTS[facePosition.face];
    rv.z = (float)FACE_TO_WORLD_MULTS[facePosition.face].y;
    return rv;
}

i32v3 VoxelSpaceConversions::getCoordinateMapping(const ChunkPosition2D& facePosition) {
    return VOXEL_TO_WORLD[facePosition.face];
}
i32v3 VoxelSpaceConversions::getCoordinateMapping(const ChunkPosition3D& facePosition) {
    return VOXEL_TO_WORLD[facePosition.face];
}

ChunkPosition2D VoxelSpaceConversions::voxelToChunk(const VoxelPosition2D& voxelPosition) {
    ChunkPosition2D gpos;
    gpos.face = voxelPosition.face;
    gpos.pos.x = fastFloor(voxelPosition.pos.x / CHUNK_WIDTH);
    gpos.pos.y = fastFloor(voxelPosition.pos.y / CHUNK_WIDTH);
    return gpos;
}
ChunkPosition3D VoxelSpaceConversions::voxelToChunk(const VoxelPosition3D& voxelPosition) {
    ChunkPosition3D gpos;
    gpos.face = voxelPosition.face;
    gpos.pos.x = fastFloor(voxelPosition.pos.x / CHUNK_WIDTH);
    gpos.pos.y = fastFloor(voxelPosition.pos.y / CHUNK_WIDTH);
    gpos.pos.z = fastFloor(voxelPosition.pos.z / CHUNK_WIDTH);
    return gpos;
}

i32v3 VoxelSpaceConversions::voxelToChunk(const i32v3& voxelPosition) {
    i32v3 gpos;
    gpos.x = fastFloor(voxelPosition.x / (float)CHUNK_WIDTH);
    gpos.y = fastFloor(voxelPosition.y / (float)CHUNK_WIDTH);
    gpos.z = fastFloor(voxelPosition.z / (float)CHUNK_WIDTH);
    return gpos;
}

i32v3 VoxelSpaceConversions::voxelToChunk(const f64v3& voxelPosition) {
    i32v3 gpos;
    gpos.x = fastFloor(voxelPosition.x / CHUNK_WIDTH);
    gpos.y = fastFloor(voxelPosition.y / CHUNK_WIDTH);
    gpos.z = fastFloor(voxelPosition.z / CHUNK_WIDTH);
    return gpos;
}

VoxelPosition2D VoxelSpaceConversions::chunkToVoxel(const ChunkPosition2D& gridPosition) {
    VoxelPosition2D vpos;
    vpos.face = gridPosition.face;
    vpos.pos.x = gridPosition.pos.x * CHUNK_WIDTH;
    vpos.pos.y = gridPosition.pos.y * CHUNK_WIDTH;
    return vpos;
}
VoxelPosition3D VoxelSpaceConversions::chunkToVoxel(const ChunkPosition3D& gridPosition) {
    VoxelPosition3D vpos;
    vpos.face = gridPosition.face;
    vpos.pos.x = gridPosition.pos.x * CHUNK_WIDTH;
    vpos.pos.y = gridPosition.pos.y * CHUNK_WIDTH;
    vpos.pos.z = gridPosition.pos.z * CHUNK_WIDTH;
    return vpos;
}

f64v3 VoxelSpaceConversions::voxelToWorld(const VoxelPosition2D& facePosition, f64 voxelWorldRadius) {
    return voxelToWorldNormalized(facePosition, voxelWorldRadius) * voxelWorldRadius;
}
f64v3 VoxelSpaceConversions::voxelToWorld(const VoxelPosition3D& facePosition, f64 voxelWorldRadius) {
    return voxelToWorldNormalized(facePosition, voxelWorldRadius) * (voxelWorldRadius + facePosition.pos.y);
}

f64v3 VoxelSpaceConversions::chunkToWorld(const ChunkPosition2D& facePosition, f64 voxelWorldRadius) {
    return chunkToWorldNormalized(facePosition, voxelWorldRadius) * voxelWorldRadius;
}
f64v3 VoxelSpaceConversions::chunkToWorld(const ChunkPosition3D& facePosition, f64 voxelWorldRadius) {
    return chunkToWorldNormalized(facePosition, voxelWorldRadius) * (voxelWorldRadius + facePosition.pos.y * CHUNK_WIDTH);
}

f64v3 VoxelSpaceConversions::voxelToWorldNormalized(const VoxelPosition2D& facePosition, f64 voxelWorldRadius) {
    const i32v3& axisMapping = VOXEL_TO_WORLD[facePosition.face];
    const i32v2& mults = FACE_TO_WORLD_MULTS[facePosition.face];

    f64v3 worldPosition;
    worldPosition[axisMapping.x] = facePosition.pos.x * mults.x;
    worldPosition[axisMapping.y] = voxelWorldRadius * FACE_Y_MULTS[facePosition.face];
    worldPosition[axisMapping.z] = facePosition.pos.y * mults.y;

    return vmath::normalize(worldPosition);
}
f64v3 VoxelSpaceConversions::voxelToWorldNormalized(const VoxelPosition3D& facePosition, f64 voxelWorldRadius) {
    const i32v3& axisMapping = VOXEL_TO_WORLD[facePosition.face];
    const i32v2& mults = FACE_TO_WORLD_MULTS[facePosition.face];

    f64v3 worldPosition;
    worldPosition[axisMapping.x] = facePosition.pos.x * mults.x;
    worldPosition[axisMapping.y] = voxelWorldRadius * FACE_Y_MULTS[facePosition.face];
    worldPosition[axisMapping.z] = facePosition.pos.z * mults.y;

    return vmath::normalize(worldPosition);
}

f64v3 VoxelSpaceConversions::chunkToWorldNormalized(const ChunkPosition2D& facePosition, f64 voxelWorldRadius) {
    return voxelToWorldNormalized(chunkToVoxel(facePosition), voxelWorldRadius);
}
f64v3 VoxelSpaceConversions::chunkToWorldNormalized(const ChunkPosition3D& facePosition, f64 voxelWorldRadius) {
    return voxelToWorldNormalized(chunkToVoxel(facePosition), voxelWorldRadius);
}

VoxelPosition3D computeGridPosition(const f32v3& hitpoint, f32 radius) {
    f32v3 cornerPos[2];
    f32 min;
    f32v3 start = vmath::normalize(hitpoint) * 2.0f * radius;
    f32v3 dir = -vmath::normalize(hitpoint);
    cornerPos[0] = f32v3(-radius, -radius, -radius);
    cornerPos[1] = f32v3(radius, radius, radius);
    if (!IntersectionUtils::boxIntersect(cornerPos, dir,
        start, min)) {
        std::cerr << "ERROR: Failed to find grid position in computeGridPosition.\n";
    }

    f32v3 gridHit = start + dir * min;
    const f32 eps = 32.0f;

    VoxelPosition3D gridPos;

    if (abs(gridHit.x - (-radius)) < eps) { //-X
        gridPos.face = WorldCubeFace::FACE_LEFT;
        gridPos.pos.x = gridHit.z;
        gridPos.pos.z = -gridHit.y;
    } else if (abs(gridHit.x - radius) < eps) { //X
        gridPos.face = WorldCubeFace::FACE_RIGHT;
        gridPos.pos.x = -gridHit.z;
        gridPos.pos.z = -gridHit.y;
    } else if (abs(gridHit.y - (-radius)) < eps) { //-Y
        gridPos.face = WorldCubeFace::FACE_BOTTOM;
        gridPos.pos.x = gridHit.x;
        gridPos.pos.z = -gridHit.z;
    } else if (abs(gridHit.y - radius) < eps) { //Y
        gridPos.face = WorldCubeFace::FACE_TOP;
        gridPos.pos.x = gridHit.x;
        gridPos.pos.z = gridHit.z;
    } else if (abs(gridHit.z - (-radius)) < eps) { //-Z
        gridPos.face = WorldCubeFace::FACE_BACK;
        gridPos.pos.x = -gridHit.x;
        gridPos.pos.z = -gridHit.y;
    } else if (abs(gridHit.z - radius) < eps) { //Z
        gridPos.face = WorldCubeFace::FACE_FRONT;
        gridPos.pos.x = gridHit.x;
        gridPos.pos.z = -gridHit.y;
    } else {
        std::cerr << "ERROR: Failed to pick voxel position in computeGridPosition.\n";
    }

    return gridPos;
}

VoxelPosition3D VoxelSpaceConversions::worldToVoxel(const f64v3& worldPosition, f64 voxelWorldRadius) {
    f64v3 wpoint = vmath::normalize(worldPosition) * voxelWorldRadius * 2.0;

    // Compute the intersection
    f32v3 normal, hitpoint;
    f32 distance;

    VoxelPosition3D gridPos;
    if (IntersectionUtils::sphereIntersect(-f32v3(vmath::normalize(wpoint)), f32v3(wpoint), f32v3(0.0f), (f32)voxelWorldRadius, hitpoint, distance, normal)) {
     
        // Compute face and grid position
        gridPos = computeGridPosition(hitpoint, (f32)voxelWorldRadius);
        gridPos.pos.y = vmath::length(worldPosition) - voxelWorldRadius;
    }

    return gridPos;
}