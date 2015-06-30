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
    int hIndex;
    int depth;

    ui16 blockID;
    ui16 tertiaryData;
    //double CaveDensity1[9][5][5], CaveDensity2[9][5][5];

    std::vector<BlockLayer>& blockLayers = m_genData->blockLayers;
    VoxelPosition3D voxPosition = chunk->getVoxelPosition();
    chunk->numBlocks = 0;

    // Generation data
    IntervalTree<ui16>::LNode blockDataArray[CHUNK_SIZE];
    IntervalTree<ui16>::LNode tertiaryDataArray[CHUNK_SIZE];
    size_t blockDataSize = 0;
    size_t tertiaryDataSize = 0;

    ui16 c = 0;

    ui32 layerIndices[CHUNK_LAYER];

    // First pass at y = 0. We separate it so we can getBlockLayerIndex a single
    // time and cut out some comparisons.
    for (size_t z = 0; z < CHUNK_WIDTH; z++) {
        for (size_t x = 0; x < CHUNK_WIDTH; x++, c++) {
            tertiaryData = 0;

            mapHeight = heightData[c].height;
            temperature = heightData[c].temperature;
            rainfall = heightData[c].rainfall;

            //tooSteep = (flags & TOOSTEEP) != 0;

            height = voxPosition.pos.y;
            depth = mapHeight - height; // Get depth of voxel

            // Get the block ID
            layerIndices[c] = getBlockLayerIndex(depth);
            BlockLayer& layer = blockLayers[layerIndices[c]];
            blockID = getBlockID(depth, mapHeight, height, layer);

            if (blockID != 0) chunk->numBlocks++;

            // Set up the data arrays
            if (blockDataSize == 0) {
                blockDataArray[blockDataSize++].set(c, 1, blockID);
                tertiaryDataArray[tertiaryDataSize++].set(c, 1, tertiaryData);
            } else {
                if (blockID == blockDataArray[blockDataSize - 1].data) {
                    blockDataArray[blockDataSize - 1].length++;
                } else {
                    blockDataArray[blockDataSize++].set(c, 1, blockID);
                }
                if (tertiaryData == tertiaryDataArray[tertiaryDataSize - 1].data) {
                    tertiaryDataArray[tertiaryDataSize - 1].length++;
                } else {
                    tertiaryDataArray[tertiaryDataSize++].set(c, 1, tertiaryData);
                }
            }
        }
    }

    // All the rest of the layers.
    for (size_t y = 1; y < CHUNK_WIDTH; y++) {
        for (size_t z = 0; z < CHUNK_WIDTH; z++) {
            for (size_t x = 0; x < CHUNK_WIDTH; x++, c++) {
                tertiaryData = 0;
                hIndex = (c & 0x3FF); // Same as % CHUNK_LAYER

                mapHeight = heightData[hIndex].height;
                temperature = heightData[hIndex].temperature;
                rainfall = heightData[hIndex].rainfall;

                height = y + voxPosition.pos.y;
                depth = mapHeight - height; // Get depth of voxel

                // Check for going up one layer
                ui16 layerIndex = layerIndices[hIndex];
                if (blockLayers[layerIndex].start > depth && layerIndex > 0) layerIndex--;
                // Get the block ID
                BlockLayer& layer = blockLayers[layerIndex];
                blockID = getBlockID(depth, mapHeight, height, layer);

                //if (tooSteep) dh += 3; // If steep, increase depth

                // TODO: Modulate dh with noise

                // TODO(Ben): Check for underground
                
                if (blockID != 0) chunk->numBlocks++;

                // Add to the data arrays
                if (blockID == blockDataArray[blockDataSize - 1].data) {
                    blockDataArray[blockDataSize - 1].length++;
                } else {
                    blockDataArray[blockDataSize++].set(c, 1, blockID);
                }
                if (tertiaryData == tertiaryDataArray[tertiaryDataSize - 1].data) {
                    tertiaryDataArray[tertiaryDataSize - 1].length++;
                } else {
                    tertiaryDataArray[tertiaryDataSize++].set(c, 1, tertiaryData);
                }
            }
        }
    }
    // Set up interval trees
    chunk->m_blocks.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, blockDataArray, blockDataSize);
    chunk->m_tertiary.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, tertiaryDataArray, tertiaryDataSize);
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

// Gets layer in O(log(n)) where n is the number of layers
ui32 ProceduralChunkGenerator::getBlockLayerIndex(ui32 depth) const {
    auto& layers = m_genData->blockLayers;

    // Binary search
    ui32 lower = 0;
    ui32 upper = layers.size() - 1;
    ui32 pos = (lower + upper) / 2;

    while (lower <= upper) {
        if (layers[pos].start <= depth && layers[pos].start + layers[pos].width > depth) {
            // We are in this layer
            return pos;
        } else if (layers[pos].start > depth) {
            upper = pos - 1;
        } else {
            lower = pos + 1;
        }
        pos = (lower + upper) / 2;
    }
    // Just return lowest layer if we fail
    return layers.size() - 1;
}

ui16 ProceduralChunkGenerator::getBlockID(int depth, int mapHeight, int height, BlockLayer& layer) const {
    ui16 blockID = 0;
    if (depth >= 0) {
        // TODO(Ben): Optimize
        blockID = 43;// layer.block;
        // Check for surface block replacement
        if (depth == 0) {
            if (blockID == m_genData->blockLayers[0].block && m_genData->surfaceBlock) {
                blockID = 43/*m_genData->surfaceBlock*/;
            }
        }
    } else {
        // Liquid
        if (height < 0 && m_genData->liquidBlock) {
            blockID = m_genData->liquidBlock;
        }
    }
    return blockID;
}
