#pragma once
class ChunkGrid;
class Chunk;

// Returns True For Certain Block Type
typedef bool(*PredBlockID)(const i32& blockID);

// Queryable Information
class VoxelRayQuery {
public:
    // Block ID
    i32 id;

    // Location Of The Picked Block
    i32v3 location;
    f32 distance;

    // Address Information
    Chunk* chunk;
    i32 voxelIndex;
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
    static const VoxelRayQuery getQuery(const f64v3& pos, const f32v3& dir, f64 maxDistance, ChunkGrid* cm, PredBlockID f);

    // Resolve A Voxel Query Keeping Previous Query Information
    static const VoxelRayFullQuery getFullQuery(const f64v3& pos, const f32v3& dir, f64 maxDistance, ChunkGrid* cm, PredBlockID f);
};