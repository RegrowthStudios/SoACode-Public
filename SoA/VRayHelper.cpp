#include "stdafx.h"
#include "VRayHelper.h"

#include "Chunk.h"
#include "ChunkManager.h"
#include "OpenglManager.h"
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

    // Get chunk position
    i32v3 chunkPos = cm->getChunkPosition(query.location);

    // TODO: Use A Bounding Box Intersection First And Allow Traversal Beginning Outside The Voxel World

    // Loop Traversal
    while (true) {
       
        relativeLocation = query.location - relativeChunkSpot;
        query.chunk = cm->getChunk(chunkPos);
      
        if (query.chunk && query.chunk->isAccessible) {
            // Calculate Voxel Index
            query.voxelIndex =
                relativeLocation.x % CHUNK_WIDTH +
                (relativeLocation.y % CHUNK_WIDTH) * CHUNK_LAYER +
                (relativeLocation.z % CHUNK_WIDTH) * CHUNK_WIDTH;

            // Get Block ID
            query.id = query.chunk->getBlockID(query.voxelIndex);

            // Check For The Block ID
            if (f(query.id)) return query;
        }
        
        // Traverse To The Next
        query.location = vr.getNextVoxelPosition();
        query.distance = vr.getDistanceTraversed();
        if (query.distance > maxDistance) return query;
        chunkPos = cm->getChunkPosition(query.location);
    }
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

    i32v3 chunkPos = cm->getChunkPosition(query.inner.location);

    // TODO: Use A Bounding Box Intersection First And Allow Traversal Beginning Outside The Voxel World

    // Loop Traversal
    while (true) {

        query.inner.chunk = cm->getChunk(chunkPos);
       
        if (query.inner.chunk && query.inner.chunk->isAccessible) {

            relativeLocation = query.inner.location - relativeChunkSpot;
            // Calculate Voxel Index
            query.inner.voxelIndex =
                relativeLocation.x % CHUNK_WIDTH +
                (relativeLocation.y % CHUNK_WIDTH) * CHUNK_LAYER +
                (relativeLocation.z % CHUNK_WIDTH) * CHUNK_WIDTH;

            // Get Block ID
            query.inner.id = query.inner.chunk->getBlockID(query.inner.voxelIndex);

            // Check For The Block ID
            if (f(query.inner.id)) return query;

            // Refresh Previous Query
            query.outer = query.inner;
        }
       
        // Traverse To The Next
        query.inner.location = vr.getNextVoxelPosition();
        query.inner.distance = vr.getDistanceTraversed();
        if (query.inner.distance > maxDistance) return query;
        chunkPos = cm->getChunkPosition(query.inner.location);
    }
}

