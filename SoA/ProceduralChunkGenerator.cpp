#include "stdafx.h"
#include "ProceduralChunkGenerator.h"

#include "NChunk.h"
#include "Constants.h"
#include "VoxelSpaceConversions.h"

#include "SmartVoxelContainer.hpp"

void ProceduralChunkGenerator::init(PlanetGenData* genData) {
    m_genData = genData;
    m_heightGenerator.init(genData);
}

void ProceduralChunkGenerator::generateChunk(NChunk* chunk, PlanetHeightData* heightData) const {

    int temperature;
    int rainfall;
    int h;
    int maph;
    int hindex;
    int dh;

    ui16 blockData;
    ui16 lampData;
    ui8 sunlightData;
    ui16 tertiaryData;
    //double CaveDensity1[9][5][5], CaveDensity2[9][5][5];

    VoxelPosition3D voxPosition = chunk->getVoxelPosition();

    // Grab the handles to the arrays
    std::vector<IntervalTree<ui16>::LNode> blockDataArray;
    std::vector<IntervalTree<ui16>::LNode> lampLightDataArray;
    std::vector<IntervalTree<ui8>::LNode> sunlightDataArray;
    std::vector<IntervalTree<ui16>::LNode> tertiaryDataArray;
    ui16 c = 0;
    for (size_t y = 0; y < CHUNK_WIDTH; y++) {
        for (size_t z = 0; z < CHUNK_WIDTH; z++) {
            for (size_t x = 0; x < CHUNK_WIDTH; x++, c++) {
                hindex = (c%CHUNK_LAYER); // TODO(Ben): Don't need modulus

                blockData = 0;
                sunlightData = 0;
                lampData = 0;
                tertiaryData = 0;

                //snowDepth = heightMap[hindex].snowDepth;
                //sandDepth = heightMap[hindex].sandDepth;
                maph = heightData[hindex].height;
                //biome = heightMap[hindex].biome;
                temperature = heightData[hindex].temperature;
                rainfall = heightData[hindex].rainfall;
               // flags = heightMap[hindex].flags;

                //tooSteep = (flags & TOOSTEEP) != 0;

                h = y + voxPosition.pos.y;
                dh = maph - h; // Get depth of voxel

                //if (tooSteep) dh += 3; // If steep, increase depth

                // TODO: Modulate dh with noise

                // Check for underground
                if (dh >= 0) {
                    //chunk->numBlocks++;
                    // TODO(Ben): Optimize
                    blockData = 2; // calculateBlockLayer((ui32)dh, genData).block;
                    // Check for surface block replacement
                    if (dh == 0) {
                        if (blockData == m_genData->blockLayers[0].block && m_genData->surfaceBlock) {
                            blockData = m_genData->surfaceBlock;
                        }
                    }
                } else {
                    // Above heightmap

                    // Liquid
                    if (h < 0 && m_genData->liquidBlock) {
                        blockData = m_genData->liquidBlock;
                        // TODO(Ben): Precalculate light here based on depth?
                    } else {
                        blockData = 0;
                        sunlightData = 31;
                    }
                }

               // if (GETBLOCK(blockData).spawnerVal || GETBLOCK(blockData).sinkVal) {
              //      chunk->spawnerBlocks.push_back(c);
              //  }

                // Set up the data arrays
                if (blockDataArray.size() == 0) {
                    blockDataArray.emplace_back(c, 1, blockData);
                    lampLightDataArray.emplace_back(c, 1, lampData);
                    sunlightDataArray.emplace_back(c, 1, sunlightData);
                    tertiaryDataArray.emplace_back(c, 1, tertiaryData);
                } else {
                    if (blockData == blockDataArray.back().data) {
                        blockDataArray.back().length++;
                    } else {
                        blockDataArray.emplace_back(c, 1, blockData);
                    }
                    if (lampData == lampLightDataArray.back().data) {
                        lampLightDataArray.back().length++;
                    } else {
                        lampLightDataArray.emplace_back(c, 1, lampData);
                    }
                    if (sunlightData == sunlightDataArray.back().data) {
                        sunlightDataArray.back().length++;
                    } else {
                        sunlightDataArray.emplace_back(c, 1, sunlightData);
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
    chunk->m_blocks.initFromSortedArray(vvox::VoxelStorageState::FLAT_ARRAY, blockDataArray);
    chunk->m_lamp.initFromSortedArray(vvox::VoxelStorageState::FLAT_ARRAY, lampLightDataArray);
    chunk->m_sunlight.initFromSortedArray(vvox::VoxelStorageState::FLAT_ARRAY, sunlightDataArray);
    chunk->m_tertiary.initFromSortedArray(vvox::VoxelStorageState::FLAT_ARRAY, tertiaryDataArray);
}

void ProceduralChunkGenerator::generateHeightmap(NChunk* chunk, PlanetHeightData* heightData) const {
    VoxelPosition3D cornerPos3D = VoxelSpaceConversions::chunkToVoxel(chunk->getChunkPosition());
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