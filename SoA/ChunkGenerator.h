#pragma once

class Chunk;

class ChunkGenerator {
public:
    static bool generateChunk(Chunk* chunk, struct LoadData *ld);
    static void TryEnqueueTree(Chunk* chunk, struct Biome *biome, int x, int z, int c);
    static void LoadMinerals(Chunk* chunk);
    static void MakeMineralVein(Chunk* chunk, struct MineralData *md, int seed);
};