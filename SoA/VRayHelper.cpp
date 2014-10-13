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

    VoxelRay vr(f32v3(pos), dir);

    // Create The Query At The Current Position
    VoxelRayQuery query = {};
    
    query.location = vr.getNextVoxelPosition();
    query.distance = vr.getDistanceTraversed();
    

    i32v3 chunkPos = cm->getChunkPosition(query.location);

    // TODO: Use A Bounding Box Intersection First And Allow Traversal Beginning Outside The Voxel World

    openglManager.debugRenderer->drawLine(f32v3(pos), f32v3(pos) + dir * 100.0f, glm::vec4(0.0f, 1.0f, 0.0f, 0.8f), 10.0);

    // Loop Traversal
    while (true) {
        openglManager.debugRenderer->drawCube(f32v3(query.location), glm::vec3(1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.5f), 10.0);


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
        chunkPos = cm->getChunkPosition(query.location);
    }
}
const VoxelRayFullQuery VRayHelper::getFullQuery(const f64v3& pos, const f32v3& dir, ChunkManager* cm, PredBlockID f) {
    // First Convert To Voxel Coordinates
    VoxelRay vr(f32v3(pos), dir);

    // Create The Query At The Current Position
    VoxelRayFullQuery query = {};
    query.inner.location = vr.getNextVoxelPosition();
    query.inner.distance = vr.getDistanceTraversed();
    query.outer.location = query.inner.location;
    query.outer.distance = query.inner.distance;

    i32v3 chunkPos = cm->getChunkPosition(query.inner.location);

    // TODO: Use A Bounding Box Intersection First And Allow Traversal Beginning Outside The Voxel World

    // Loop Traversal
    while (true) {

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
        chunkPos = cm->getChunkPosition(query.inner.location);
    }
}

