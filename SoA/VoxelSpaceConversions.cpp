#include "stdafx.h"
#include "VoxelSpaceConversions.h"

f32v3 VoxelSpaceConversions::getCoordinateMults(const ChunkFacePosition2D& facePosition) {
    f32v3 rv;
    rv.x = (float)FACE_TO_WORLD_MULTS[facePosition.face][0].x;
    rv.y = (float)FACE_Y_MULTS[facePosition.face];
    rv.z = (float)FACE_TO_WORLD_MULTS[facePosition.face][0].y;
    return rv;
}
f32v3 VoxelSpaceConversions::getCoordinateMults(const ChunkFacePosition3D& facePosition) {
    f32v3 rv;
    rv.x = (float)FACE_TO_WORLD_MULTS[facePosition.face][0].x;
    rv.y = (float)FACE_Y_MULTS[facePosition.face];
    rv.z = (float)FACE_TO_WORLD_MULTS[facePosition.face][0].y;
    return rv;
}

i32v3 VoxelSpaceConversions::getCoordinateMapping(const ChunkFacePosition2D& facePosition) {
    return VOXEL_TO_WORLD[facePosition.face];
}
i32v3 VoxelSpaceConversions::getCoordinateMapping(const ChunkFacePosition3D& facePosition) {
    return VOXEL_TO_WORLD[facePosition.face];
}

ChunkPosition2D VoxelSpaceConversions::voxelGridToChunkGrid(const VoxelPosition2D& voxelPosition) {
    ChunkPosition2D gpos;
    gpos.face = voxelPosition.face;
    gpos.rotation = voxelPosition.rotation;
    gpos.pos.x = fastFloor(voxelPosition.pos.x / CHUNK_WIDTH);
    gpos.pos.y = fastFloor(voxelPosition.pos.y / CHUNK_WIDTH);
    return gpos;
}
ChunkPosition3D VoxelSpaceConversions::voxelGridToChunkGrid(const VoxelPosition3D& voxelPosition) {
    ChunkPosition3D gpos;
    gpos.face = voxelPosition.face;
    gpos.rotation = voxelPosition.rotation;
    gpos.pos.x = fastFloor(voxelPosition.pos.x / CHUNK_WIDTH);
    gpos.pos.y = fastFloor(voxelPosition.pos.y / CHUNK_WIDTH);
    gpos.pos.z = fastFloor(voxelPosition.pos.z / CHUNK_WIDTH);
    return gpos;
}

VoxelPosition2D VoxelSpaceConversions::chunkGridToVoxelGrid(const ChunkPosition2D& gridPosition) {
    VoxelPosition2D vpos;
    vpos.face = gridPosition.face;
    vpos.rotation = gridPosition.rotation;
    vpos.pos.x = gridPosition.pos.x * CHUNK_WIDTH;
    vpos.pos.y = gridPosition.pos.y * CHUNK_WIDTH;
    return vpos;
}
VoxelPosition3D VoxelSpaceConversions::chunkGridToVoxelGrid(const ChunkPosition3D& gridPosition) {
    VoxelPosition3D vpos;
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

VoxelFacePosition2D VoxelSpaceConversions::voxelGridToFace(const VoxelPosition2D& gridPosition) {
    const i32v2& mult = GRID_TO_FACE_MULTS[gridPosition.rotation];

    VoxelFacePosition2D facePos;
    facePos.face = gridPosition.face;
    facePos.pos = gridPosition.pos * f64v2(mult);

    if (gridPosition.rotation % 2) { //when rotation%2 x and z must switch
        std::swap(facePos.pos.x, facePos.pos.y);
    }

    return facePos;
}
VoxelFacePosition3D VoxelSpaceConversions::voxelGridToFace(const VoxelPosition3D& gridPosition) {
    const i32v2& mult = GRID_TO_FACE_MULTS[gridPosition.rotation];

    VoxelFacePosition3D facePos;
    facePos.face = gridPosition.face;
    facePos.pos = f64v3(gridPosition.pos.x * mult.x, gridPosition.pos.y, gridPosition.pos.z * mult.y);

    if (gridPosition.rotation % 2) { //when rotation%2 x and z must switch
        std::swap(facePos.pos.x, facePos.pos.z);
    }

    return facePos;
}

ChunkFacePosition2D VoxelSpaceConversions::chunkGridToFace(const ChunkPosition2D& gridPosition) {
    const i32v2& mult = GRID_TO_FACE_MULTS[gridPosition.rotation];

    ChunkFacePosition2D facePos;
    facePos.face = gridPosition.face;
    facePos.pos = gridPosition.pos * mult;

    if (gridPosition.rotation % 2) { //when rotation%2 x and z must switch
        std::swap(facePos.pos.x, facePos.pos.y);
    }

    return facePos;
}
ChunkFacePosition3D VoxelSpaceConversions::chunkGridToFace(const ChunkPosition3D& gridPosition) {
    const i32v2& mult = GRID_TO_FACE_MULTS[gridPosition.rotation];

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
    const i32v3& axisMapping = VOXEL_TO_WORLD[facePosition.face];
    const i32v2& mults = FACE_TO_WORLD_MULTS[facePosition.face][0];

    f64v3 worldPosition;
    worldPosition[axisMapping.x] = facePosition.pos.x * mults.x;
    worldPosition[axisMapping.y] = voxelWorldRadius * FACE_Y_MULTS[facePosition.face];
    worldPosition[axisMapping.z] = facePosition.pos.y * mults.y;

    return glm::normalize(worldPosition);
}
f64v3 VoxelSpaceConversions::voxelFaceToWorldNormalized(const VoxelFacePosition3D& facePosition, f64 voxelWorldRadius) {
    const i32v3& axisMapping = VOXEL_TO_WORLD[facePosition.face];
    const i32v2& mults = FACE_TO_WORLD_MULTS[facePosition.face][0];

    f64v3 worldPosition;
    worldPosition[axisMapping.x] = facePosition.pos.x * mults.x;
    worldPosition[axisMapping.y] = voxelWorldRadius * FACE_Y_MULTS[facePosition.face];
    worldPosition[axisMapping.z] = facePosition.pos.z * mults.y;

    return glm::normalize(worldPosition);
}

f64v3 VoxelSpaceConversions::chunkFaceToWorldNormalized(const ChunkFacePosition2D& facePosition, f64 voxelWorldRadius) {
    return voxelFaceToWorldNormalized(chunkFaceToVoxelFace(facePosition), voxelWorldRadius);
}
f64v3 VoxelSpaceConversions::chunkFaceToWorldNormalized(const ChunkFacePosition3D& facePosition, f64 voxelWorldRadius) {
    return voxelFaceToWorldNormalized(chunkFaceToVoxelFace(facePosition), voxelWorldRadius);
}

VoxelPosition3D VoxelSpaceConversions::worldToVoxelGrid(const f64v3& worldPosition, f64 voxelWorldRadius) {
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