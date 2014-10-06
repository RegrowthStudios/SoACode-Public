#include "stdafx.h"
#include "VRayHelper.h"

#include "Chunk.h"
#include "ChunkManager.h"
#include "OpenglManager.h"
#include "Player.h"
#include "VoxelRay.h"

const VoxelRayQuery VRayHelper::getQuery(const f64v3& pos, const f32v3& dir, ChunkManager* cm, PredBlockID f) {
    // First Convert To Voxel Coordinates
    //TODO(Cristian) use new mapping for this
    f64v3 relativePosition = pos;// -f64v3(cm->cornerPosition);
    VoxelRay vr(f32v3(relativePosition), dir);

    // Create The Query At The Current Position
    VoxelRayQuery query = {};
    query.location = vr.getNextVoxelPosition();
    query.distance = vr.getDistanceTraversed();
    i32v3 chunkPos = query.location / CHUNK_WIDTH;

    // TODO: Use A Bounding Box Intersection First And Allow Traversal Beginning Outside The Voxel World

    // Loop Traversal
    while (true) {
        // Make Sure Ray Stays In Bounds
        if (!cm->isChunkPositionInBounds(chunkPos))
            return query;

        query.chunk = cm->getChunk(chunkPos);
      
        if (query.chunk && query.chunk->isAccessible) {
            // Calculate Voxel Index
            query.voxelIndex =
                query.location.x % CHUNK_WIDTH +
                (query.location.y % CHUNK_WIDTH) * CHUNK_LAYER +
                (query.location.z % CHUNK_WIDTH) * CHUNK_WIDTH;

            // Get Block ID
            query.id = query.chunk->getBlockID(query.voxelIndex);

            // Check For The Block ID
            if (f(query.id)) return query;
        }
        
        // Traverse To The Next
        query.location = vr.getNextVoxelPosition();
        query.distance = vr.getDistanceTraversed();
        chunkPos = query.location / CHUNK_WIDTH;
    }
}
const VoxelRayFullQuery VRayHelper::getFullQuery(const f64v3& pos, const f32v3& dir, ChunkManager* cm, PredBlockID f) {
    // First Convert To Voxel Coordinates
    //TODO(Cristian) use new mapping for this
    f64v3 relativePosition = pos;// -f64v3(cm->cornerPosition);
    VoxelRay vr(f32v3(relativePosition), dir);

    // Create The Query At The Current Position
    VoxelRayFullQuery query = {};
    query.inner.location = vr.getNextVoxelPosition();
    query.inner.distance = vr.getDistanceTraversed();
    query.outer.location = query.inner.location;
    query.outer.distance = query.inner.distance;
    i32v3 chunkPos = query.inner.location / CHUNK_WIDTH;

    // TODO: Use A Bounding Box Intersection First And Allow Traversal Beginning Outside The Voxel World

    // Loop Traversal
    while (true) {
        // Make Sure Ray Stays In Bounds
        if (!cm->isChunkPositionInBounds(chunkPos))
            return query;

        query.inner.chunk = cm->getChunk(chunkPos);
       
        if (query.inner.chunk && query.inner.chunk->isAccessible) {
            // Calculate Voxel Index
            query.inner.voxelIndex =
                query.inner.location.x % CHUNK_WIDTH +
                (query.inner.location.y % CHUNK_WIDTH) * CHUNK_LAYER +
                (query.inner.location.z % CHUNK_WIDTH) * CHUNK_WIDTH;

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
        chunkPos = query.inner.location / CHUNK_WIDTH;
    }
}

