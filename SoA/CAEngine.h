#pragma once
#include "LiquidData.h"
#include "Constants.h"

class ChunkManager;
class Chunk;

class CAEngine {
public:
    CAEngine();
    void setChunk(Chunk* chunk) { _chunk = chunk; }
    void updateSpawnerBlocks(bool powders);
    void updateLiquidBlocks();
    void updatePowderBlocks();
private:
    
    void liquidPhysics(i32 startBlockIndex, i32 b);
    void powderPhysics(i32 c);
    void snowPhysics(i32 c);
    void lockChunk(Chunk* chunk);

    i32 _lowIndex;
    i32 _range;
    i32 _highIndex;
    std::vector<ui16> _usedUpdateFlagList;
    bool _blockUpdateFlagList[CHUNK_SIZE];
    Chunk* _chunk;
    Chunk* _lockedChunk;
};