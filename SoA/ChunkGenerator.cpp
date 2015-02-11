#include "stdafx.h"


#include "Biome.h"
#include "BlockData.h"
#include "ChunkGenerator.h"
#include "PlanetData.h"

#include <Vorb/Timing.h>

#include "BlockPack.h"
#include "Chunk.h"
#include "WorldStructs.h"
#include "VoxelIntervalTree.h"
#include "GameManager.h"

bool ChunkGenerator::generateChunk(Chunk* chunk, class LoadData *ld)
{
    PreciseTimer timer;
    timer.start();
    const HeightData *heightMap = ld->heightMap;
    const PlanetGenData* genData = ld->genData;

    Biome *biome;
    chunk->numBlocks = 0;
    int temperature;
    int rainfall;
    double CaveDensity1[9][5][5], CaveDensity2[9][5][5];

    // Grab the handles to the arrays
    std::vector<VoxelIntervalTree<ui16>::LightweightNode> blockDataArray;
    std::vector<VoxelIntervalTree<ui16>::LightweightNode> lampLightDataArray;
    std::vector<VoxelIntervalTree<ui8>::LightweightNode> sunlightDataArray;
    std::vector<VoxelIntervalTree<ui16>::LightweightNode> tertiaryDataArray;

    int c = 0;
    int flags;
    int h = 0, maph;
    int hindex;
    int snowDepth, sandDepth;
    double r;
    int needsCave = 3;
    bool needsSurface = 1;
    int pnum;
    int dh;
    double ti, tj;
    bool tooSteep;

    chunk->minh = chunk->voxelPosition.y - heightMap[CHUNK_LAYER / 2].height; //pick center height as the overall height for minerals

    ui16 blockData;
    ui16 lampData;
    ui8 sunlightData;
    ui16 tertiaryData;

    for (size_t y = 0; y < CHUNK_WIDTH; y++){
        pnum = chunk->numBlocks;
        for (size_t z = 0; z < CHUNK_WIDTH; z++) {
            for (size_t x = 0; x < CHUNK_WIDTH; x++, c++) {

                hindex = (c%CHUNK_LAYER);

                blockData = 0;
                sunlightData = 0;
                lampData = 0;
                tertiaryData = 0;

                snowDepth = heightMap[hindex].snowDepth;
                sandDepth = heightMap[hindex].sandDepth;
                maph = heightMap[hindex].height;
                biome = heightMap[hindex].biome;
                temperature = heightMap[hindex].temperature;
                rainfall = heightMap[hindex].rainfall;
                flags = heightMap[hindex].flags;

                tooSteep = (flags & TOOSTEEP) != 0;

                h = y + chunk->voxelPosition.y;
                dh = maph - h; // Get depth of voxel

                if (tooSteep) dh += 3; // If steep, increase depth

                // TODO: Modulate dh with noise

                // Check for underground
                if (dh >= 0) {
                    chunk->numBlocks++;
                    // TODO(Ben): Optimize
                    blockData = calculateBlockLayer(dh, genData).block;
                    // Check for surface block replacement
                    if (dh == 0) {
                        if (blockData == genData->blockLayers[0].block && genData->surfaceBlock) {
                            blockData = genData->surfaceBlock;
                        }
                    }
                } else {
                    // Above heightmap

                    // Liquid
                    if (h < 0 && genData->liquidBlock) {
                        blockData = genData->liquidBlock;
                        // TODO(Ben): Precalculate light here based on depth?
                    } else {
                        blockData = NONE;
                        sunlightData = 31;
                    }
                }
                
                if (GETBLOCK(blockData).spawnerVal || GETBLOCK(blockData).sinkVal){
                    chunk->spawnerBlocks.push_back(c); 
                }

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
        //if (pnum == chunk->numBlocks && maph - h < 0){
        //    // TODO(Ben): test this
        //    blockDataArray.back().length += CHUNK_SIZE - c;
        //    lampLightDataArray.back().length += CHUNK_SIZE - c;
        //    sunlightDataArray.back().length += CHUNK_SIZE - c;
        //    tertiaryDataArray.back().length += CHUNK_SIZE - c; 
        //    break;
        //}
    }

    // Set up interval trees
    chunk->_blockIDContainer.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, blockDataArray);
    chunk->_lampLightContainer.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, lampLightDataArray);
    chunk->_sunlightContainer.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, sunlightDataArray);
    chunk->_tertiaryDataContainer.initFromSortedArray(vvox::VoxelStorageState::INTERVAL_TREE, tertiaryDataArray);

    if (chunk->numBlocks){
        LoadMinerals(chunk);
    }

    return (chunk->numBlocks != 0);
}

void ChunkGenerator::TryEnqueueTree(Chunk* chunk, Biome *biome, int x, int z, int c)
{
 
    /*int index = FloraGenerator::getTreeIndex(biome, x, z);
    if (index == -1) return;
    chunk->treesToLoad.emplace_back();
    chunk->treesToLoad.back().startc = c;
    FloraGenerator::makeTreeData(chunk, chunk->treesToLoad.back(), GameManager::planet->treeTypeVec[index]);*/
}

void ChunkGenerator::LoadMinerals(Chunk* chunk)
{
    const int minh = chunk->minh;
    float chance;
    float d;
    MineralData *md;
    for (Uint32 i = 0; i < chunk->possibleMinerals.size(); i++){
        md = chunk->possibleMinerals[i];
        if (minh > md->centerHeight && minh <= md->startHeight){
            chance = ((((float)md->startHeight - minh) / ((float)md->startHeight - md->centerHeight))*(md->centerChance - md->startChance)) + md->startChance;
        } else if (minh <= md->centerHeight && minh >= md->endHeight){
            chance = ((((float)minh - md->endHeight) / ((float)md->centerHeight - md->endHeight))*(md->centerChance - md->endChance)) + md->endChance;
        } else{
            chance = 0;
        }
        d = (float)(PseudoRand(chunk->voxelPosition.x + chunk->voxelPosition.y - i*i * 11, chunk->voxelPosition.z + 8 * i - 2 * chunk->voxelPosition.y) + 1.0)*50.0;

        if (d <= chance - 10.0){ //3 ore veins
            MakeMineralVein(chunk, md, 32);
            MakeMineralVein(chunk, md, 6433);
            MakeMineralVein(chunk, md, 9189);
        } else if (d <= chance - 5.0){ //2 ore veins
            MakeMineralVein(chunk, md, 53);
            MakeMineralVein(chunk, md, 2663);
        } else if (d <= chance){
            MakeMineralVein(chunk, md, 882);
        }
    }
}

void ChunkGenerator::MakeMineralVein(Chunk* chunk, MineralData *md, int seed)
{
    int c = (int)(((PseudoRand(chunk->voxelPosition.x - seed*seed + 3 * chunk->voxelPosition.y + md->blockType * 2, chunk->voxelPosition.z + seed * 4 - chunk->voxelPosition.y + md->blockType - 44) + 1.0) / 2.0)*CHUNK_SIZE);
    int btype = md->blockType;
    int size = ((PseudoRand(chunk->voxelPosition.x + 2 * chunk->voxelPosition.y - md->blockType * 4 + seed, chunk->voxelPosition.z + chunk->voxelPosition.y - md->blockType + 44) + 1.0) / 2.0)*(md->maxSize - md->minSize) + md->minSize;
    int r;
    int x, y, z;
    for (int i = 0; i < size; i++){
        
        // hack to stop generating minerals in the air
        int blockID = chunk->getBlockID(c);
        if (blockID && blockID != DIRTGRASS && blockID < LOWWATER) {
            chunk->setBlockData(c, btype);
        }

        x = c % CHUNK_WIDTH;
        y = c / CHUNK_LAYER;
        z = (c % CHUNK_LAYER) / CHUNK_WIDTH;

        r = (int)((PseudoRand(chunk->voxelPosition.x * c + c * i + btype + seed * 2, c * c - chunk->voxelPosition.z + 6 - chunk->voxelPosition.y * btype - i * 3 - seed) + 1.0) * 2.5 + 0.5); //0-5
        if (r == 0 && y > 0){
            c -= CHUNK_LAYER;
        } else if (r == 1 && x > 0){
            c--;
        } else if (r == 2 && x < CHUNK_WIDTH - 1){
            c++;
        } else if (r == 3 && z > 0){
            c -= CHUNK_WIDTH;
        } else if (r == 4 && z < CHUNK_WIDTH - 1){
            c += CHUNK_WIDTH;
        } else if (y < CHUNK_WIDTH - 1){
            c += CHUNK_LAYER;
        } else{ //down by default
            c -= CHUNK_LAYER;
        }
    }
}

// TODO(Ben): Only need to do this once per col or even once per chunk
const BlockLayer& ChunkGenerator::calculateBlockLayer(int depth, const PlanetGenData* genData) {
    auto& layers = genData->blockLayers;

    // Binary search
    int lower = 0;
    int upper = layers.size()-1;
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
