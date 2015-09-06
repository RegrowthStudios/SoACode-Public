#pragma once
#include "Constants.h"

// TODO(Ben): Temporary
#include "BlockData.h"
#include "Chunk.h"

class PhysicsEngine;
class ChunkManager;
enum class ChunkStates;

class ChunkUpdater {
public:
    static void randomBlockUpdates(PhysicsEngine* physicsEngine, Chunk* chunk);
    static void placeBlock(Chunk* chunk, Chunk*& lockedChunk, BlockIndex blockIndex, BlockID blockData) {
        placeBlockNoUpdate(chunk, blockIndex, blockData);
        //addBlockToUpdateList(chunk, lockedChunk, blockIndex);
    }
    static void placeBlockSafe(Chunk* chunk, Chunk*& lockedChunk, BlockIndex blockIndex, BlockID blockData);
    static void placeBlockNoUpdate(Chunk* chunk, BlockIndex blockIndex, BlockID blockType);
    static void placeBlockFromLiquidPhysics(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int blockType);
    static void placeBlockFromLiquidPhysicsSafe(Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int blockType);
  
    static void removeBlock(ChunkManager* chunkManager, PhysicsEngine* physicsEngine, Chunk* chunk, Chunk*& lockedChunk, int blockIndex, bool isBreak, double force = 0.0, f32v3 explodeDir = f32v3(0.0f));
    static void removeBlockSafe(ChunkManager* chunkManager, PhysicsEngine* physicsEngine, Chunk* chunk, Chunk*& lockedChunk, int blockIndex, bool isBreak, double force = 0.0, f32v3 explodeDir = f32v3(0.0f));
    static void removeBlockFromLiquidPhysics(Chunk* chunk, Chunk*& lockedChunk, int blockIndex);
    static void removeBlockFromLiquidPhysicsSafe(Chunk* chunk, Chunk*& lockedChunk, int blockIndex);

    static void updateNeighborStates(Chunk* chunk, const i32v3& pos, ChunkStates state);
    static void updateNeighborStates(Chunk* chunk, int blockID, ChunkStates state);

    static void addBlockToUpdateList(Chunk* chunk, Chunk*& lockedChunk, int c);
    static void snowAddBlockToUpdateList(Chunk* chunk, int c);

private:
    //TODO: Replace with emitterOnBreak
    static void breakBlock(Chunk* chunk, int x, int y, int z, int blockType, double force = 0.0f, f32v3 extraForce = f32v3(0.0f));

    static void placeFlora(Chunk* chunk, int blockIndex, int blockID);
    static void removeFlora(ChunkManager* chunkManager, PhysicsEngine* physicsEngine, Chunk* chunk, Chunk*& lockedChunk, int blockIndex, int blockID);

    //Fire
    static void updateFireBlock(ChunkManager* chunkManager, PhysicsEngine* physicsEngine, Chunk* chunk, int blockIndex);
    static float getBurnProbability(Chunk* chunk, Chunk*& lockedChunk, int blockIndex);
    static void burnAdjacentBlocks(ChunkManager* chunkManager, PhysicsEngine* physicsEngine, Chunk* chunk, Chunk*& lockedChunk, int blockIndex);
    static inline void checkBurnBlock(int blockIndex, Chunk*& lockedChunk, int blockType, Chunk *owner, float burnMult = 1.0);
};