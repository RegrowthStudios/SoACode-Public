#pragma once
class ChunkGrid;
class Chunk;

#include "BlockData.h"

// Returns True For Certain Block Type
typedef bool(*PredBlock)(const Block& block);

extern bool solidVoxelPredBlock(const Block& block);

// Queryable Information
class VoxelRayQuery {
public:
    // Block ID
    BlockID id;

    // Location Of The Picked Block
    i32v3 location;
    f64 distance;

    // Address Information
    ChunkID chunkID;
    ui16 voxelIndex;
};
class VoxelRayFullQuery {
public:
    // The Place Of The Chosen Block
    VoxelRayQuery inner;

    // The Place Before The Chosen Block
    VoxelRayQuery outer;
};

// Utility Methods For Using A Voxel Ray
class VRayHelper {
public:
    // Resolve A Simple Voxel Query
    static const VoxelRayQuery getQuery(const f64v3& pos, const f32v3& dir, f64 maxDistance, ChunkGrid& cm, PredBlock f = &solidVoxelPredBlock);

    // Resolve A Voxel Query Keeping Previous Query Information
    static const VoxelRayFullQuery getFullQuery(const f64v3& pos, const f32v3& dir, f64 maxDistance, ChunkGrid& cm, PredBlock f = &solidVoxelPredBlock);
};