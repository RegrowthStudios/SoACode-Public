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

    ui16 blockData;
    ui16 lampData;
    ui8 sunlightData;
    ui16 tertiaryData;
    //double CaveDensity1[9][5][5], CaveDensity2[9][5][5];

    // Grab the handles to the arrays
    std::vector<VoxelIntervalTree<ui16>::LightweightNode> blockDataArray;
    std::vector<VoxelIntervalTree<ui16>::LightweightNode> lampLightDataArray;
    std::vector<VoxelIntervalTree<ui8>::LightweightNode> sunlightDataArray;
    std::vector<VoxelIntervalTree<ui16>::LightweightNode> tertiaryDataArray;
    ui16 c = 0;
    for (size_t y = 0; y < CHUNK_WIDTH; y++) {
        for (size_t z = 0; z < CHUNK_WIDTH; z++) {
            for (size_t x = 0; x < CHUNK_WIDTH; x++, c++) {
                blockData = 0;
                sunlightData = 0;
                lampData = 0;
                tertiaryData = 0;

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
    chunk->m_blocks.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, blockDataArray);
    chunk->m_lamp.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, lampLightDataArray);
    chunk->m_sunlight.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, sunlightDataArray);
    chunk->m_tertiary.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, tertiaryDataArray);
}

void ProceduralChunkGenerator::generateHeightmap(NChunk* chunk, PlanetHeightData* heightData) const {
    VoxelPosition3D cornerPos3D = VoxelSpaceConversions::chunkToVoxel(chunk->getPosition());
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