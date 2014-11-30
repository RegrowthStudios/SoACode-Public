#pragma once

#include "types.h"

class Chunk;

class ChunkGenerator {
public:
    static bool generateChunk(Chunk* chunk, class LoadData *ld);
    static void TryEnqueueTree(Chunk* chunk, Biome *biome, int x, int z, int c);
    static void LoadMinerals(Chunk* chunk);
    static void MakeMineralVein(Chunk* chunk, MineralData *md, int seed);
};