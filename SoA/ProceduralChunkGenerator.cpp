#include "stdafx.h"
#include "ProceduralChunkGenerator.h"

#include "Chunk.h"
#include "Constants.h"
#include "VoxelSpaceConversions.h"

#include "SmartVoxelContainer.hpp"

void ProceduralChunkGenerator::init(PlanetGenData* genData) {
    m_genData = genData;
    m_heightGenerator.init(genData);
}

void ProceduralChunkGenerator::generateChunk(Chunk* chunk, PlanetHeightData* heightData) const {

    int temperature;
    int rainfall;
    int height;
    int mapHeight;
    int hindex;
    int depth;

    ui16 blockData;
    ui16 tertiaryData;
    //double CaveDensity1[9][5][5], CaveDensity2[9][5][5];

    VoxelPosition3D voxPosition = chunk->getVoxelPosition();
    chunk->numBlocks = 0;

    // Grab the handles to the arrays
    std::vector<IntervalTree<ui16>::LNode> blockDataArray;
    std::vector<IntervalTree<ui16>::LNode> tertiaryDataArray;
    ui16 c = 0;
    for (size_t y = 0; y < CHUNK_WIDTH; y++) {
        for (size_t z = 0; z < CHUNK_WIDTH; z++) {
            for (size_t x = 0; x < CHUNK_WIDTH; x++, c++) {
                hindex = (c%CHUNK_LAYER); // TODO(Ben): Don't need modulus

                blockData = 0;
                tertiaryData = 0;

                //snowDepth = heightMap[hindex].snowDepth;
                //sandDepth = heightMap[hindex].sandDepth;
                mapHeight = heightData[hindex].height;
                //biome = heightMap[hindex].biome;
                temperature = heightData[hindex].temperature;
                rainfall = heightData[hindex].rainfall;
               // flags = heightMap[hindex].flags;

                //tooSteep = (flags & TOOSTEEP) != 0;

                height = y + voxPosition.pos.y;
                depth = mapHeight - height; // Get depth of voxel

                //if (tooSteep) dh += 3; // If steep, increase depth

                // TODO: Modulate dh with noise

                // Check for underground
                if (depth >= 0) {
                    //chunk->numBlocks++;
                    // TODO(Ben): Optimize
                    blockData = getBlockLayer(depth).block;
                    // Check for surface block replacement
                    if (depth == 0) {
                        if (blockData == m_genData->blockLayers[0].block && m_genData->surfaceBlock) {
                            blockData = 43/*m_genData->surfaceBlock*/;
                        }
                    }
                } else {
                    // Liquid
                    if (height < 0 && m_genData->liquidBlock) {
                        blockData = m_genData->liquidBlock;
                    }
                }

                // TODO(Ben): Just for mesh testing purposes
                //if ((int)(voxPosition.pos.y / 32) % 6 == 0) {
                //    if (y < 2 /*|| (y < 5 && x % 8 == 0 && z % 8 == 0)*/) {
                //        blockData = 43;
                //    }
                //    if (x % 6 == 0 && z % 6 == 0 && y == 2) {
                //        blockData = 43;
                //    }
                //    if (x % 6 == 3 && z % 6 == 3 && y == 1) {
                //        blockData = 0;
                //    }
                //}
                //if ((int)(voxPosition.pos.y / 32) % 6 == 0) {
                //    if ((x < 5 && z < 5)) {
                //        blockData = 43;
                //    }
                //}
                //if ((int)(voxPosition.pos.y / 32 + 1) % 6 == 0) {
                //    if ((x < 4 && z < 4 && x > 0 && z > 0)) {
                //        blockData = 43;
                //    }
                //}
                //if ((int)(voxPosition.pos.y / 32 + 5) % 6 == 0) {
                //    if ((x < 5 && z < 5)) {
                //        blockData = 43;
                //    }
                //}
                //if ((int)(voxPosition.pos.y / 32 + 4) % 6 == 0) {
                //    if ((x < 4 && z < 4 && x > 0 && z > 0)) {
                //        blockData = 43;
                //    }
                //}
                //if ((x < 3 && z < 3 && x > 1 && z > 1)) {
                //    blockData = 43;
                //}
                
                if (blockData != 0) chunk->numBlocks++;

                // Set up the data arrays
                if (blockDataArray.size() == 0) {
                    blockDataArray.emplace_back(c, 1, blockData);
                    tertiaryDataArray.emplace_back(c, 1, tertiaryData);
                } else {
                    if (blockData == blockDataArray.back().data) {
                        blockDataArray.back().length++;
                    } else {
                        blockDataArray.emplace_back(c, 1, blockData);
                    }
                    if (tertiaryData == tertiaryDataArray.back().data) {
                        tertiaryDataArray.back().length++;
                    } else {
                        tertiaryDataArray.emplace_back(c, 1, tertiaryData);
                    }
                }
            }
        }
    }
    // Set up interval trees
    chunk->m_blocks.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, blockDataArray);
    chunk->m_tertiary.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, tertiaryDataArray);
}

void ProceduralChunkGenerator::generateHeightmap(Chunk* chunk, PlanetHeightData* heightData) const {
    VoxelPosition3D cornerPos3D = chunk->getVoxelPosition();
    VoxelPosition2D cornerPos2D;
    cornerPos2D.pos.x = cornerPos3D.pos.x;
    cornerPos2D.pos.y = cornerPos3D.pos.z;
    cornerPos2D.face = cornerPos3D.face;
    for (int z = 0; z < CHUNK_WIDTH; z++) {
        for (int x = 0; x < CHUNK_WIDTH; x++) {
            VoxelPosition2D pos = cornerPos2D;
            pos.pos.x += x;
            pos.pos.y += z;
            m_heightGenerator.generateHeight(heightData[z * CHUNK_WIDTH + x], pos);
        }
    }
}

// TODO(Ben): Only need to do this once per col or even once per chunk
const BlockLayer& ProceduralChunkGenerator::getBlockLayer(int depth) const {
    auto& layers = m_genData->blockLayers;

    // Binary search
    int lower = 0;
    int upper = layers.size() - 1;
    int pos = (lower + upper) / 2;

    while (lower <= upper) {
        if (layers[pos].start <= depth && layers[pos].start + layers[pos].width > depth) {
            // We are in this layer
            return layers[pos];
        } else if (layers[pos].start > depth) {
            upper = pos - 1;
        } else {
            lower = pos + 1;
        }
        pos = (lower + upper) / 2;
    }
    // Just return lowest layer if we fail
    return layers.back();
}