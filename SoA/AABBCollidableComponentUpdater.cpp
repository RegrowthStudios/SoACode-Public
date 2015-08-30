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
    cmp.collisions.clear();
    // Get needed components
    auto& physics = gameSystem->physics.get(cmp.physics);
    auto& position = gameSystem->voxelPosition.get(physics.voxelPosition);
    if (position.parentVoxel == 0) return;
    auto& sphericalVoxel = spaceSystem->sphericalVoxel.get(position.parentVoxel);
    f64v3 vpos = position.gridPosition.pos + f64v3(cmp.offset - cmp.box * 0.5f);
    i32v3 vp(vmath::floor(vpos));
    
    i32v3 bounds(vmath::ceil(f64v3(cmp.box) + vmath::fract(vpos)));
    ChunkGrid& grid = sphericalVoxel.chunkGrids[position.gridPosition.face];
    const BlockPack* bp = sphericalVoxel.blockPack;

    // TODO(Ben): Optimize
    for (int yi = 0; yi < bounds.y; yi++) {
        for (int zi = 0; zi < bounds.z; zi++) {
            for (int xi = 0; xi < bounds.x; xi++) {
                i32v3 p = vp + i32v3(xi, yi, zi);
                i32v3 cpos = VoxelSpaceConversions::voxelToChunk(p);
                // TODO(Ben): Negative numbers?
                ChunkID chunkID(cpos);
                ChunkHandle chunk = grid.accessor.acquire(chunkID);
                if (chunk->genLevel == GEN_DONE) {
                    i32v3 cp = p - cpos * CHUNK_WIDTH;
                    ui16 index = cp.y * CHUNK_LAYER + cp.z * CHUNK_WIDTH + cp.x;
                    if (bp->operator[](chunk->blocks.get(index)).collide) {
                        cmp.collisions[chunkID].voxels.push_back(index);
                    }
                }
                chunk.release();
            }
        }
    }
}
