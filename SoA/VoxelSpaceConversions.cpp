#include "stdafx.h"
#include "VoxelSpaceConversions.h"

f32v3 VoxelSpaceConversions::getCoordinateMults(const ChunkPosition2D& facePosition) {
    f32v3 rv;
    rv.x = (float)FACE_TO_WORLD_MULTS[facePosition.face][0].x;
    rv.y = (float)FACE_Y_MULTS[facePosition.face];
    rv.z = (float)FACE_TO_WORLD_MULTS[facePosition.face][0].y;
    return rv;
}
f32v3 VoxelSpaceConversions::getCoordinateMults(const ChunkPosition3D& facePosition) {
    f32v3 rv;
    rv.x = (float)FACE_TO_WORLD_MULTS[facePosition.face][0].x;
    rv.y = (float)FACE_Y_MULTS[facePosition.face];
    rv.z = (float)FACE_TO_WORLD_MULTS[facePosition.face][0].y;
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
    const i32v2& mults = FACE_TO_WORLD_MULTS[facePosition.face][0];

    f64v3 worldPosition;
    worldPosition[axisMapping.x] = facePosition.pos.x * mults.x;
    worldPosition[axisMapping.y] = voxelWorldRadius * FACE_Y_MULTS[facePosition.face];
    worldPosition[axisMapping.z] = facePosition.pos.y * mults.y;

    return glm::normalize(worldPosition);
}
f64v3 VoxelSpaceConversions::voxelToWorldNormalized(const VoxelPosition3D& facePosition, f64 voxelWorldRadius) {
    const i32v3& axisMapping = VOXEL_TO_WORLD[facePosition.face];
    const i32v2& mults = FACE_TO_WORLD_MULTS[facePosition.face][0];

    f64v3 worldPosition;
    worldPosition[axisMapping.x] = facePosition.pos.x * mults.x;
    worldPosition[axisMapping.y] = voxelWorldRadius * FACE_Y_MULTS[facePosition.face];
    worldPosition[axisMapping.z] = facePosition.pos.z * mults.y;

    return glm::normalize(worldPosition);
}

f64v3 VoxelSpaceConversions::chunkToWorldNormalized(const ChunkPosition2D& facePosition, f64 voxelWorldRadius) {
    return voxelToWorldNormalized(chunkToVoxel(facePosition), voxelWorldRadius);
}
f64v3 VoxelSpaceConversions::chunkToWorldNormalized(const ChunkPosition3D& facePosition, f64 voxelWorldRadius) {
    return voxelToWorldNormalized(chunkToVoxel(facePosition), voxelWorldRadius);
}

VoxelPosition3D VoxelSpaceConversions::worldToVoxel(const f64v3& worldPosition, f64 voxelWorldRadius) {
    f64v3 boxIntersect;
    VoxelPosition3D gpos;

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