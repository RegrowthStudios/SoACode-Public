#pragma once

class Chunk;

class ChunkGenerator {
public:
    static bool generateChunk(Chunk* chunk, class LoadData *ld);
    static void TryEnqueueTree(Chunk* chunk, class Biome *biome, int x, int z, int c);

    static void LoadMinerals(Chunk* chunk);
    static void MakeMineralVein(Chunk* chunk, struct MineralData *md, int seed);
private:
    /// Determines which rock layer is at a given depth in O(logn)
    /// @param depth: The depth underground
    /// @param genData: The planet generation data
    /// @return the containing layer
    static const BlockLayer& calculateBlockLayer(int depth, const PlanetGenData* genData);
};