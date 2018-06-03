#include "stdafx.h"
#include "VRayHelper.h"

#include "BlockPack.h"
#include "ChunkGrid.h"
#include "VoxelRay.h"
#include "VoxelSpaceConversions.h"

bool solidVoxelPredBlock(const Block& block) {
    return block.collide == true;
}

const VoxelRayQuery VRayHelper::getQuery(const f64v3& pos, const f32v3& dir, f64 maxDistance, ChunkGrid& cg, PredBlock f) {

    // Set the ray coordinates
    VoxelRay vr(pos, f64v3(dir));

    // Create The Query At The Current Position
    VoxelRayQuery query = {};
    
    query.location = vr.getNextVoxelPosition();
    query.distance = vr.getDistanceTraversed();

    // A minimum chunk position for determining voxel coords using only positive numbers
    i32v3 relativeChunkSpot = VoxelSpaceConversions::voxelToChunk(f64v3(pos.x - maxDistance, pos.y - maxDistance, pos.z - maxDistance)) * CHUNK_WIDTH;
    i32v3 relativeLocation;

    // Chunk position
    i32v3 chunkPos;

    // TODO: Use A Bounding Box Intersection First And Allow Traversal Beginning Outside The Voxel World

    // Keep track of the previous chunk for locking
    ChunkHandle chunk;
    query.chunkID = ChunkID(0xffffffffffffffff);
    bool locked = false;

    // Loop Traversal
    while (query.distance < maxDistance) {
       
        chunkPos = VoxelSpaceConversions::voxelToChunk(query.location);

        relativeLocation = query.location - relativeChunkSpot;
        ChunkID id(chunkPos);
        if (id != query.chunkID) {
            query.chunkID = id;
            if (chunk.isAquired()) {
                if (locked) {
                    chunk->dataMutex.unlock();
                    locked = false;
                }
                chunk.release();
            }
            chunk = cg.accessor.acquire(id);
            if (chunk->isAccessible) {
                chunk->dataMutex.lock();
                locked = true;
            }
        }
        
        if (locked) {
            // Calculate Voxel Index
            query.voxelIndex =
                (relativeLocation.x & 0x1f) +
                (relativeLocation.y & 0x1f) * CHUNK_LAYER +
                (relativeLocation.z & 0x1f) * CHUNK_WIDTH;

            // Get Block ID
            query.id = chunk->blocks.get(query.voxelIndex);

            // Check For The Block ID
            if (f(cg.blockPack->operator[](query.id))) {
                if (locked) chunk->dataMutex.unlock();
                chunk.release();
                return query;
            }
        }
        
        // Traverse To The Next
        query.location = vr.getNextVoxelPosition();
        query.distance = vr.getDistanceTraversed();
    }
    if (chunk.isAquired()) {
        if (locked) chunk->dataMutex.unlock();
        chunk.release();
    }
    return query;
}
const VoxelRayFullQuery VRayHelper::getFullQuery(const f64v3& pos, const f32v3& dir, f64 maxDistance, ChunkGrid& cg, PredBlock f) {
    // First Convert To Voxel Coordinates
    VoxelRay vr(pos, f64v3(dir));

    // Create The Query At The Current Position
    VoxelRayFullQuery query = {};
    query.inner.location = vr.getNextVoxelPosition();
    query.inner.distance = vr.getDistanceTraversed();
    query.outer.location = query.inner.location;
    query.outer.distance = query.inner.distance;

    // A minimum chunk position for determining voxel coords using only positive numbers
    i32v3 relativeChunkSpot = VoxelSpaceConversions::voxelToChunk(f64v3(pos.x - maxDistance, pos.y - maxDistance, pos.z - maxDistance)) * CHUNK_WIDTH;
    i32v3 relativeLocation;

    i32v3 chunkPos;

    // TODO: Use A Bounding Box Intersection First And Allow Traversal Beginning Outside The Voxel World

    // Keep track of the previous chunk for locking
    ChunkHandle chunk;
    query.inner.chunkID = ChunkID(0xffffffffffffffff);
    bool locked = false;

    // Loop Traversal
    while (query.inner.distance < maxDistance) {

        chunkPos = VoxelSpaceConversions::voxelToChunk(query.inner.location);
        ChunkID id(chunkPos);
       
        if (id != query.inner.chunkID) {
            query.inner.chunkID = id;
            if (chunk.isAquired()) {
                if (locked) {
                    chunk->dataMutex.unlock();
                    locked = false;
                }
                chunk.release();
            }
            chunk = cg.accessor.acquire(id);
            if (chunk->isAccessible) {
                chunk->dataMutex.lock();
                locked = true;
            }
        }
       
        if (locked) {
            relativeLocation = query.inner.location - relativeChunkSpot;
            // Calculate Voxel Index
            query.inner.voxelIndex =
                (relativeLocation.x & 0x1f) +
                (relativeLocation.y & 0x1f) * CHUNK_LAYER +
                (relativeLocation.z & 0x1f) * CHUNK_WIDTH;

            // Get Block ID
            query.inner.id = chunk->blocks.get(query.inner.voxelIndex);

            // Check For The Block ID
            if (f(cg.blockPack->operator[](query.inner.id))) {
                if (locked) chunk->dataMutex.unlock();
                chunk.release();
                return query;
            }

            // Refresh Previous Query
            query.outer = query.inner;
        }
       
        // Traverse To The Next
        query.inner.location = vr.getNextVoxelPosition();
        query.inner.distance = vr.getDistanceTraversed();
    }
    if (chunk.isAquired()) {
        if (locked) chunk->dataMutex.unlock();
        chunk.release();
    }
    return query;
}

