#include "stdafx.h"
#include "VRayHelper.h"

#include "Chunk.h"
#include "ChunkManager.h"
#include "Player.h"
#include "VoxelRay.h"

const VoxelRayQuery VRayHelper::getQuery(const f64v3& pos, const f32v3& dir, f64 maxDistance, ChunkManager* cm, PredBlockID f) {

    // Set the ray coordinates
    VoxelRay vr(pos, f64v3(dir));

    // Create The Query At The Current Position
    VoxelRayQuery query = {};
    
    query.location = vr.getNextVoxelPosition();
    query.distance = vr.getDistanceTraversed();

    // A minimum chunk position for determining voxel coords using only positive numbers
    i32v3 relativeChunkSpot = cm->getChunkPosition(f64v3(pos.x - maxDistance, pos.y - maxDistance, pos.z - maxDistance)) * CHUNK_WIDTH;
    i32v3 relativeLocation;

    // Chunk position
    i32v3 chunkPos;

    // TODO: Use A Bounding Box Intersection First And Allow Traversal Beginning Outside The Voxel World

    // Keep track of the previous chunk for locking
    Chunk* prevChunk = nullptr;

    // Loop Traversal
    while (query.distance < maxDistance) {
       
        chunkPos = cm->getChunkPosition(query.location);

        relativeLocation = query.location - relativeChunkSpot;
        query.chunk = cm->getChunk(chunkPos);
      
        if (query.chunk && query.chunk->isAccessible) {
            // Check for if we need to lock the next chunk
            if (prevChunk != query.chunk) {
                if (prevChunk) prevChunk->unlock();
                query.chunk->lock();
                prevChunk = query.chunk;
            }

            // Calculate Voxel Index
            query.voxelIndex =
                relativeLocation.x % CHUNK_WIDTH +
                (relativeLocation.y % CHUNK_WIDTH) * CHUNK_LAYER +
                (relativeLocation.z % CHUNK_WIDTH) * CHUNK_WIDTH;

            // Get Block ID
            query.id = query.chunk->getBlockID(query.voxelIndex);

            // Check For The Block ID
            if (f(query.id)) {
                if (prevChunk) prevChunk->unlock();
                return query;
            }
        }
        
        // Traverse To The Next
        query.location = vr.getNextVoxelPosition();
        query.distance = vr.getDistanceTraversed();
    }
    if (prevChunk) prevChunk->unlock();
    return query;
}
const VoxelRayFullQuery VRayHelper::getFullQuery(const f64v3& pos, const f32v3& dir, f64 maxDistance, ChunkManager* cm, PredBlockID f) {
    // First Convert To Voxel Coordinates
    VoxelRay vr(pos, f64v3(dir));

    // Create The Query At The Current Position
    VoxelRayFullQuery query = {};
    query.inner.location = vr.getNextVoxelPosition();
    query.inner.distance = vr.getDistanceTraversed();
    query.outer.location = query.inner.location;
    query.outer.distance = query.inner.distance;

    // A minimum chunk position for determining voxel coords using only positive numbers
    i32v3 relativeChunkSpot = cm->getChunkPosition(f64v3(pos.x - maxDistance, pos.y - maxDistance, pos.z - maxDistance)) * CHUNK_WIDTH;
    i32v3 relativeLocation;

    i32v3 chunkPos;

    // TODO: Use A Bounding Box Intersection First And Allow Traversal Beginning Outside The Voxel World

    // Keep track of the previous chunk for locking
    Chunk* prevChunk = nullptr;

    // Loop Traversal
    while (query.inner.distance < maxDistance) {

        chunkPos = cm->getChunkPosition(query.inner.location);

        query.inner.chunk = cm->getChunk(chunkPos);
       
        if (query.inner.chunk && query.inner.chunk->isAccessible) {
            // Check for if we need to lock the next chunk
            if (prevChunk != query.inner.chunk) {
                if (prevChunk) prevChunk->unlock();
                query.inner.chunk->lock();
                prevChunk = query.inner.chunk;
            }

            relativeLocation = query.inner.location - relativeChunkSpot;
            // Calculate Voxel Index
            query.inner.voxelIndex =
                relativeLocation.x % CHUNK_WIDTH +
                (relativeLocation.y % CHUNK_WIDTH) * CHUNK_LAYER +
                (relativeLocation.z % CHUNK_WIDTH) * CHUNK_WIDTH;

            // Get Block ID
            query.inner.id = query.inner.chunk->getBlockID(query.inner.voxelIndex);

            // Check For The Block ID
            if (f(query.inner.id)) {
                if (prevChunk) prevChunk->unlock();
                return query;
            }

            // Refresh Previous Query
            query.outer = query.inner;
        }
       
        // Traverse To The Next
        query.inner.location = vr.getNextVoxelPosition();
        query.inner.distance = vr.getDistanceTraversed();
    }
    if (prevChunk) prevChunk->unlock();
    return query;
}

