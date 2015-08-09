#include "stdafx.h"
#include "ChunkSphereComponentUpdater.h"

#include "ChunkAccessor.h"
#include "ChunkID.h"
#include "soaUtils.h"
#include "VoxelSpaceConversions.h"

#define GET_INDEX(x, y, z) (((x) + cmp.radius) + ((y) + cmp.radius) * cmp.layer + ((z) + cmp.radius) * cmp.width)

void ChunkSphereComponentUpdater::update(GameSystem* gameSystem) {
    for (auto& it : gameSystem->chunkSphere) {
        auto& cmp = it.second;
        auto& voxelPos = gameSystem->voxelPosition.get(cmp.voxelPosition);
        ChunkPosition3D chunkPos = VoxelSpaceConversions::voxelToChunk(voxelPos.gridPosition);
        // Check for shift
        if (chunkPos.pos != cmp.centerPosition) {
            i32v3 offset = chunkPos.pos - cmp.centerPosition;

            i32 dx2 = offset.x * offset.x;
            i32 dy2 = offset.y * offset.y;
            i32 dz2 = offset.z * offset.z;
            if (dx2 + dy2 + dz2 == 1) {
                // Fast version. Shift by 1
                if (dx2) {
                    if (offset.x > 0) {
                        std::cout << "PX ";
                        // Release
                        for (auto& o : cmp.acquireOffsets) {
                            i32v3 p;
                            p.x = cmp.offset.x - o.x + 1;
                            p.y = cmp.offset.y + o.y;
                            p.z = cmp.offset.z + o.z;
                            if (p.x < -cmp.radius) p.x += cmp.width;
                            cmp.handleGrid[GET_INDEX(p.x, p.y, p.z)].release();
                        }
                        // Acquire
                        for (auto& o : cmp.acquireOffsets) {
                            i32v3 p = cmp.offset + o;
                            if (p.x > cmp.radius) p.x -= cmp.width;
                            ChunkID id;
                            id.x = cmp.centerPosition.x + o.x;
                            id.y = cmp.centerPosition.y + o.y;
                            id.z = cmp.centerPosition.z + o.z;
                            cmp.handleGrid[GET_INDEX(p.x, p.y, p.z)] = cmp.accessor->acquire(id);
                        }

                        cmp.offset.x++;
                        if (cmp.offset.x > cmp.radius) cmp.offset.x = -cmp.radius;
                        cmp.centerPosition = chunkPos.pos;
                    } else {
                        std::cout << "NX ";
                        //   m_offset.x--;
                        //    if (m_offset.x < -m_radius) m_offset.x = m_radius;
                    }


                    //    m_centerPosition = agentChunkPosition;
                } else if (dz2) {
                    std::cout << "Z ";
                } else {
                    std::cout << "Y ";
                }
            } else {
                // Slow version. Multi-chunk shift.
                std::cout << "TOO BIG " << dx2 << std::endl;
            }
        }
    }
}

#undef GET_INDEX

void ChunkSphereComponentUpdater::setRadius(ChunkSphereComponent& cmp, ui32 radius) {
    // Release old handles
    for (int i = 0; i < cmp.size; i++) {
        if (cmp.handleGrid[i].isAquired()) cmp.handleGrid[i].release();
    }
    delete[] cmp.handleGrid;

    // Set vars
    cmp.radius = radius;
    cmp.width = (cmp.radius - 1) * 2 + 1;
    cmp.layer = cmp.width * cmp.width;
    cmp.size = cmp.layer * cmp.width;

    // Allocate handles
    cmp.handleGrid = new ChunkHandle[cmp.size];

    // Pre-compute offsets
    int radius2 = cmp.radius * cmp.radius;
    // Get all in range slots
    for (int y = -cmp.radius; y <= cmp.radius; y++) {
        for (int z = -cmp.radius; z <= cmp.radius; z++) {
            for (int x = -cmp.radius; x <= cmp.radius; x++) {
                // Check if its in range
                if (x * x + y * y + z * z < radius2) {
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
