#include "stdafx.h"
#include "AABBCollidableComponentUpdater.h"

#include "GameSystem.h"
#include "SpaceSystem.h"
#include "BlockPack.h"

#include "VoxelSpaceConversions.h"

void AABBCollidableComponentUpdater::update(GameSystem* gameSystem, SpaceSystem* spaceSystem) {
    for (auto& it : gameSystem->aabbCollidable) {
        collideWithVoxels(it.second, gameSystem, spaceSystem);
    }
}

void AABBCollidableComponentUpdater::collideWithVoxels(AabbCollidableComponent& cmp, GameSystem* gameSystem, SpaceSystem* spaceSystem) {
    // Clear old data
    cmp.voxelCollisions.clear();
    // Get needed components
    auto& physics = gameSystem->physics.get(cmp.physics);
    auto& position = gameSystem->voxelPosition.get(physics.voxelPosition);
    if (position.parentVoxel == 0) return;
    auto& sphericalVoxel = spaceSystem->sphericalVoxel.get(position.parentVoxel);
    f64v3 vpos = position.gridPosition.pos + f64v3(cmp.offset - cmp.box * 0.5f);
    i32v3 vp(glm::floor(vpos));
    
    i32v3 bounds(glm::ceil(f64v3(cmp.box) + glm::fract(vpos)));
    ChunkGrid& grid = sphericalVoxel.chunkGrids[position.gridPosition.face];
    const BlockPack* bp = sphericalVoxel.blockPack;

    std::map<ChunkID, std::vector<ui16>> boundedVoxels;

    // Get bounded voxels
    for (int yi = 0; yi < bounds.y; yi++) {
        for (int zi = 0; zi < bounds.z; zi++) {
            for (int xi = 0; xi < bounds.x; xi++) {
                i32v3 p = vp + i32v3(xi, yi, zi);
                i32v3 cpos = VoxelSpaceConversions::voxelToChunk(p);
                // TODO(Ben): Negative numbers?
                ChunkID chunkID(cpos);
                i32v3 cp = p - cpos * CHUNK_WIDTH;
                boundedVoxels[chunkID].push_back(cp.y * CHUNK_LAYER + cp.z * CHUNK_WIDTH + cp.x);
            }
        }
    }

    // Find Collidable voxels
    for (auto& it : boundedVoxels) {
        ChunkHandle chunk = grid.accessor.acquire(it.first);
        if (chunk->genLevel == GEN_DONE) {
            std::lock_guard<std::mutex> l(chunk->dataMutex);
            for (auto& i : it.second) {
                BlockID id = chunk->blocks.get(i);
                if (bp->operator[](id).collide) {
                    // TODO(Ben): Don't need to look up every time.
                    cmp.voxelCollisions[it.first].emplace_back(id, i);
                }
            }
        }
        chunk.release();
    }

    // Helper macro for below code
#define CHECK_CODE(dir) \
    /* Acquire new chunk if needed */ \
    if (id != currentID) { \
        /* Release current chunk */ \
        if (chunk.isAquired()) { \
            chunk->dataMutex.unlock(); \
            chunk.release(); \
        } \
        chunk = grid.accessor.acquire(id); \
        if (chunk->genLevel != GEN_DONE) { \
            chunk.release(); \
            currentID = ChunkID(0xffffffffffffffffu); \
        } else { \
            chunk->dataMutex.lock(); \
            currentID = id; \
            /* Check the voxel */ \
            if (chunk->genLevel == GEN_DONE && bp->operator[](chunk->blocks.get(index)).collide) { \
                cd.##dir = true; \
            } \
        } \
    } else {\
        /* Check the voxel */ \
        if (chunk->genLevel == GEN_DONE && bp->operator[](chunk->blocks.get(index)).collide) { \
            cd.##dir = true; \
        } \
    }

    // Set neighbor collide flags
    // TODO(Ben): More than top
    ChunkID currentID(0xffffffffffffffffu);
    ChunkHandle chunk;
    for (auto& it : cmp.voxelCollisions) {
        for (auto& cd : it.second) {
            { // Left
                ChunkID id = it.first;
                int index = (int)cd.index;
                if ((index & 0x1f) == 0) {
                    index += CHUNK_WIDTH_M1;
                    id.x--;
                } else {
                    index--;
                }
                CHECK_CODE(left);
            }
            { // Right
                ChunkID id = it.first;
                int index = (int)cd.index;
                if ((index & 0x1f) == CHUNK_WIDTH_M1) {
                    index -= CHUNK_WIDTH_M1;
                    id.x++;
                } else {
                    index++;
                }
                CHECK_CODE(right);
            }
            { // Bottom
                ChunkID id = it.first;
                int index = (int)cd.index;
                if (index < CHUNK_LAYER) {
                    index += (CHUNK_SIZE - CHUNK_LAYER);
                    id.y--;
                } else {
                    index -= CHUNK_LAYER;
                }
                CHECK_CODE(bottom);
            }
            { // Top
                ChunkID id = it.first;
                int index = (int)cd.index;
                if (index >= CHUNK_SIZE - CHUNK_LAYER) {
                    index -= (CHUNK_SIZE - CHUNK_LAYER);
                    id.y++;
                } else {
                    index += CHUNK_LAYER;
                }
                CHECK_CODE(top);
            }
            { // Back
                ChunkID id = it.first;
                int index = (int)cd.index;
                if ((index & 0x3ff) / CHUNK_WIDTH == 0) {
                    index += (CHUNK_LAYER - CHUNK_WIDTH);
                    id.z--;
                } else {
                    index -= CHUNK_WIDTH_M1;
                }
                CHECK_CODE(back);
            }
            { // Front
                ChunkID id = it.first;
                int index = (int)cd.index;
                if ((index & 0x3ff) / CHUNK_WIDTH == CHUNK_WIDTH_M1) {
                    index -= (CHUNK_LAYER - CHUNK_WIDTH);
                    id.z++;
                } else {
                    index += CHUNK_WIDTH_M1;
                }
                CHECK_CODE(front);
            }
        }
    }
    // Release chunk if needed
    if (chunk.isAquired()) {
        chunk->dataMutex.unlock();
        chunk.release();
    }
#undef CHECK_CODE
}
