#include "stdafx.h"
#include "ChunkSphereComponentUpdater.h"

#include "ChunkAccessor.h"
#include "ChunkID.h"
#include "GameSystem.h"
#include "SpaceSystem.h"
#include "VoxelSpaceConversions.h"
#include "soaUtils.h"

#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2

void ChunkSphereComponentUpdater::update(GameSystem* gameSystem, SpaceSystem* spaceSystem) {
    for (auto& it : gameSystem->chunkSphere) {
        ChunkSphereComponent& cmp = it.second;
        auto& voxelPos = gameSystem->voxelPosition.get(cmp.voxelPosition);
        ChunkPosition3D chunkPos = VoxelSpaceConversions::voxelToChunk(voxelPos.gridPosition);

        // Check for grid shift or init
        if (cmp.currentCubeFace != chunkPos.face) {
            releaseHandles(cmp);
            cmp.centerPosition = chunkPos;
            cmp.currentCubeFace = chunkPos.face;
            auto& sphericalVoxel = spaceSystem->sphericalVoxel.get(voxelPos.parentVoxelComponent);
            cmp.accessor = &sphericalVoxel.chunkGrids[chunkPos.face].accessor;
            initSphere(cmp);
        }

        // Check for shift
        if (chunkPos.pos != cmp.centerPosition) {
            i32v3 offset = chunkPos.pos - cmp.centerPosition;

            i32 dx2 = offset.x * offset.x;
            i32 dy2 = offset.y * offset.y;
            i32 dz2 = offset.z * offset.z;
            if (dx2 + dy2 + dz2 == 1) {
                // Hideous but fast version. Shift by 1.
                if (dx2) {
                    shiftDirection(cmp, X_AXIS, Y_AXIS, Z_AXIS, offset.x);
                } else if (dz2) {
                    shiftDirection(cmp, Z_AXIS, Y_AXIS, X_AXIS, offset.z);
                } else {
                    shiftDirection(cmp, Y_AXIS, X_AXIS, Z_AXIS, offset.y);
                }
                cmp.centerPosition = chunkPos.pos;
            } else {
                // Slow version. Multi-chunk shift.
                std::cout << "BIG SHIFT";
                cmp.centerPosition = chunkPos.pos;
                int radius2 = cmp.radius * cmp.radius;
                // TODO(Ben): Could be optimized
                cmp.offset = i32v3(0);

                // Get all in range slots
                for (int y = -cmp.radius; y <= cmp.radius; y++) {
                    for (int z = -cmp.radius; z <= cmp.radius; z++) {
                        for (int x = -cmp.radius; x <= cmp.radius; x++) {
                            // Check if its in range
                            if (x * x + y * y + z * z <= radius2) {
                                int index = (y + cmp.radius) * cmp.layer + (z + cmp.radius) * cmp.width + (x + cmp.radius);
                                ChunkID id;
                                id.x = cmp.centerPosition.x + x;
                                id.y = cmp.centerPosition.y + y;
                                id.z = cmp.centerPosition.z + z;
                                if (cmp.handleGrid[index].isAquired()) cmp.handleGrid[index].release();
                                cmp.handleGrid[index] = cmp.accessor->acquire(id);
                            }
                        }
                    }
                }
            }
        }
    }
}

void ChunkSphereComponentUpdater::setRadius(ChunkSphereComponent& cmp, ui32 radius) {
    // Release old handles
    releaseHandles(cmp);

    // Set vars
    cmp.radius = radius;
    cmp.width = cmp.radius * 2 + 1;
    cmp.layer = cmp.width * cmp.width;
    cmp.size = cmp.layer * cmp.width;

    // Allocate handles
    initSphere(cmp);
}

#define GET_INDEX(x, y, z) (((x) + cmp.radius) + ((y) + cmp.radius) * cmp.layer + ((z) + cmp.radius) * cmp.width)

void ChunkSphereComponentUpdater::shiftDirection(ChunkSphereComponent& cmp, int axis1, int axis2, int axis3, int offset) {
    if (offset > 0) {
        // Release
        for (auto& o : cmp.acquireOffsets) {
            i32v3 p;
            p[axis1] = cmp.offset[axis1] - o.x + 1;
            p[axis2] = cmp.offset[axis2] + o.y;
            p[axis3] = cmp.offset[axis3] + o.z;
            // Wrap (I hope this gets optimized)
            for (int i = 0; i < 3; i++) {
                if (p[i] < -cmp.radius) {
                    p[i] += cmp.width;
                } else if (p[i] > cmp.radius) {
                    p[i] -= cmp.width;
                }
            }
            cmp.handleGrid[GET_INDEX(p.x, p.y, p.z)].release();
        }
        // Acquire
        for (auto& o : cmp.acquireOffsets) {
            i32v3 p;
            p[axis1] = cmp.offset[axis1] + o.x;
            p[axis2] = cmp.offset[axis2] + o.y;
            p[axis3] = cmp.offset[axis3] + o.z;
            // Wrap (I hope this gets optimized)
            for (int i = 0; i < 3; i++) {
                if (p[i] < -cmp.radius) {
                    p[i] += cmp.width;
                } else if (p[i] > cmp.radius) {
                    p[i] -= cmp.width;
                }
            }
            ChunkID id;
            i32v3 off;
            off[axis1] = o.x;
            off[axis2] = o.y;
            off[axis3] = o.z;
            id.x = cmp.centerPosition.x + off.x;
            id.y = cmp.centerPosition.y + off.y;
            id.z = cmp.centerPosition.z + off.z;
            cmp.handleGrid[GET_INDEX(p.x, p.y, p.z)] = cmp.accessor->acquire(id);
        }

        cmp.offset[axis1]++;
        if (cmp.offset[axis1] > cmp.radius) cmp.offset[axis1] = -cmp.radius;
    } else {
        for (auto& o : cmp.acquireOffsets) {
            i32v3 p;
            p[axis1] = cmp.offset[axis1] + o.x - 1;
            p[axis2] = cmp.offset[axis2] + o.y;
            p[axis3] = cmp.offset[axis3] + o.z;
            // Wrap (I hope this gets optimized)
            for (int i = 0; i < 3; i++) {
                if (p[i] < -cmp.radius) {
                    p[i] += cmp.width;
                } else if (p[i] > cmp.radius) {
                    p[i] -= cmp.width;
                }
            }
            cmp.handleGrid[GET_INDEX(p.x, p.y, p.z)].release();
        }
        // Acquire
        for (auto& o : cmp.acquireOffsets) {
            i32v3 p;
            p[axis1] = cmp.offset[axis1] - o.x;
            p[axis2] = cmp.offset[axis2] + o.y;
            p[axis3] = cmp.offset[axis3] + o.z;
            // Wrap (I hope this gets optimized)
            for (int i = 0; i < 3; i++) {
                if (p[i] < -cmp.radius) {
                    p[i] += cmp.width;
                } else if (p[i] > cmp.radius) {
                    p[i] -= cmp.width;
                }
            }
            ChunkID id;
            i32v3 off;
            off[axis1] = -o.x;
            off[axis2] = o.y;
            off[axis3] = o.z;
            id.x = cmp.centerPosition.x + off.x;
            id.y = cmp.centerPosition.y + off.y;
            id.z = cmp.centerPosition.z + off.z;
            cmp.handleGrid[GET_INDEX(p.x, p.y, p.z)] = cmp.accessor->acquire(id);
        }

        cmp.offset[axis1]--;
        if (cmp.offset[axis1] < -cmp.radius) cmp.offset[axis1] = cmp.radius;
    }
}

#undef GET_INDEX

void ChunkSphereComponentUpdater::releaseHandles(ChunkSphereComponent& cmp) {
    if (cmp.handleGrid) {
        for (int i = 0; i < cmp.size; i++) {
            if (cmp.handleGrid[i].isAquired()) cmp.handleGrid[i].release();
        }
        delete[] cmp.handleGrid;
        cmp.handleGrid = nullptr;
    }
}

void ChunkSphereComponentUpdater::initSphere(ChunkSphereComponent& cmp) {
    cmp.handleGrid = new ChunkHandle[cmp.size];
    cmp.offset = i32v3(0);

    // Pre-compute offsets
    int radius2 = cmp.radius * cmp.radius;

    // Get all in range slots
    for (int y = -cmp.radius; y <= cmp.radius; y++) {
        for (int z = -cmp.radius; z <= cmp.radius; z++) {
            for (int x = -cmp.radius; x <= cmp.radius; x++) {
                // Check if its in range
                if (x * x + y * y + z * z <= radius2) {
                    int index = (y + cmp.radius) * cmp.layer + (z + cmp.radius) * cmp.width + (x + cmp.radius);
                    ChunkID id;
                    id.x = cmp.centerPosition.x + x;
                    id.y = cmp.centerPosition.y + y;
                    id.z = cmp.centerPosition.z + z;
                    cmp.handleGrid[index] = cmp.accessor->acquire(id);
                }
            }
        }
    }
    // Determine +x acquire offsets
    int index = 0;
    for (int y = -cmp.radius; y <= cmp.radius; y++) {
        for (int z = -cmp.radius; z <= cmp.radius; z++) {
            for (int x = -cmp.radius; x <= cmp.radius; x++, index++) {
                if (cmp.handleGrid[index].isAquired()) {
                    if (x == cmp.radius || !cmp.handleGrid[index + 1].isAquired()) {
                        cmp.acquireOffsets.emplace_back(x + 1, y, z);
                    }
                }
            }
        }
    }
}