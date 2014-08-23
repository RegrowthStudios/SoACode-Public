#pragma once

#include "types.h"

class Chunk;

class ChunkGenerator {
public:
    static bool generateChunk(Chunk* chunk, struct LoadData *ld);
    static void TryPlaceTree(Chunk* chunk, Biome *biome, int x, int z, int c);
    static void LoadMinerals(Chunk* chunk);
    static void MakeMineralVein(Chunk* chunk, MineralData *md, int seed);
};