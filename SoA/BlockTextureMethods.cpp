#include "stdafx.h"
#include "BlockTextureMethods.h"

#include <Vorb/graphics/ConnectedTextures.h>
#include <Vorb/utils.h>

#include "BlockPack.h"
#include "Chunk.h"
#include "ChunkMesh.h"
#include "ChunkMesher.h"
#include "VoxelBits.h"

#define GETBLOCK(a) (((*blocks)[((a) & 0x0FFF)]))
#define TEXTURE_INDEX block->textures[params.faceIndex]->layers[params.layerIndex].indices[params.typeIndex]

//Gets a random offset for use by random textures
void BlockTextureMethods::getRandomTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result) {
    //TODO: MurmurHash3
    const ChunkMesher* cm = params.chunkMesher;
    const BlockTextureLayer* blockTexInfo = params.blockTexInfo;
    i32 seed = 0; // getPositionSeed(cm->x + cm->position.x, cm->y + cm->position.y, cm->z + cm->position.z);

    f32 r = (f32)((/*PseudoRand(seed) +*/ 1.0) * 0.5 * blockTexInfo->totalWeight);
    f32 totalWeight = 0;

    // TODO(Ben): Binary search?
    if (blockTexInfo->weights.size()) {
        for (ui32 i = 0; i < blockTexInfo->numTiles; i++) {
            totalWeight += blockTexInfo->weights[i];
            if (r <= totalWeight) {
                result += i;
                return;
            }
        }
    } else {
        for (ui32 i = 0; i < blockTexInfo->numTiles; i++) {
            totalWeight += 1.0f;
            if (r <= totalWeight) {
                result += i;
                return;
            }
        }
    }
}

void BlockTextureMethods::getFloraTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result) {
    //TODO: MurmurHash3
    const ChunkMesher* cm = params.chunkMesher;
    i32 seed = 0; // getPositionSeed(cm->x + cm->position.x, cm->y + cm->position.y, cm->z + cm->position.z);

    f32 r = (f32)((/*PseudoRand(seed) +*/ 1.0) * 0.5 * params.blockTexInfo->totalWeight);
    f32 totalWeight = 0;

    const BlockTextureLayer* blockTexInfo = params.blockTexInfo;
    const ui16* tertiaryData = cm->tertiaryData;

    const int& blockIndex = cm->blockIndex;

    int column;

    // TODO(Ben): Binary search?
    if (blockTexInfo->weights.size()) {
        for (ui32 i = 0; i < blockTexInfo->size.x; i++) {
            totalWeight += blockTexInfo->weights[i];
            if (r <= totalWeight) {
                column = i;
                break;
            }
        }
    } else {
        for (ui32 i = 0; i < blockTexInfo->size.x; i++) {
            totalWeight += 1.0f;
            if (r <= totalWeight) {
                column = i;
                break;
            }
        }
    }

    result += column;

    // Get the height of the current voxel
    int height = MIN(VoxelBits::getFloraHeight(tertiaryData[blockIndex]), cm->block->floraHeight);
    int yPos = height - VoxelBits::getFloraPosition(tertiaryData[blockIndex]);

    // Move the result to the flora of the correct height
    result += blockTexInfo->size.x * (height * height + height) / 2;
    // Offset by the ypos
    result += blockTexInfo->size.x * yPos;

}

//Gets a connected texture offset by looking at the surrounding blocks
void BlockTextureMethods::getConnectedTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result) {
    const BlockPack* blocks = params.chunkMesher->blocks;
    int connectedOffset = 0;
    const int& blockIndex = params.chunkMesher->blockIndex;
    const int& upDir = params.upDir;
    const int& rightDir = params.rightDir;
    const int& frontDir = params.frontDir;
    const ui16* blockIDData = params.chunkMesher->blockData;
    BlockTextureIndex tex = result;

    // Top Left
    const Block *block = &GETBLOCK(blockIDData[blockIndex + upDir - rightDir]);

    if (TEXTURE_INDEX != tex) {
        connectedOffset |= 0x80;
    }

    // Top
    block = &GETBLOCK(blockIDData[blockIndex + upDir]);
    if (TEXTURE_INDEX != tex) {
        connectedOffset |= 0xE0;
    }

    // Top Right
    block = &GETBLOCK(blockIDData[blockIndex + upDir + rightDir]);
    if (TEXTURE_INDEX != tex) {
        connectedOffset |= 0x20;
    }

    // Right
    block = &GETBLOCK(blockIDData[blockIndex + rightDir]);
    if (TEXTURE_INDEX != tex) {
        connectedOffset |= 0x38;
    }

    // Bottom Right
    block = &GETBLOCK(blockIDData[blockIndex - upDir + rightDir]);
    if (TEXTURE_INDEX != tex) {
        connectedOffset |= 0x8;
    }

    // Bottom
    block = &GETBLOCK(blockIDData[blockIndex - upDir]);
    if (TEXTURE_INDEX != tex) {
        connectedOffset |= 0xE;
    }

    // Bottom Left
    block = &GETBLOCK(blockIDData[blockIndex - upDir - rightDir]);
    if (TEXTURE_INDEX != tex) {
        connectedOffset |= 0x2;
    }

    // Left
    block = &GETBLOCK(blockIDData[blockIndex - rightDir]);
    if (TEXTURE_INDEX != tex) {
        connectedOffset |= 0x83;
    }

    if (params.blockTexInfo->innerSeams) {
        // Top Front Left
        block = &GETBLOCK(blockIDData[blockIndex + upDir - rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x80;
        }

        // Top Front Right
        block = &GETBLOCK(blockIDData[blockIndex + upDir + rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x20;
        }

        // Bottom front Right
        block = &GETBLOCK(blockIDData[blockIndex - upDir + rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x8;
        }

        //Bottom front
        block = &GETBLOCK(blockIDData[blockIndex - upDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0xE;
        }

        // Bottom front Left
        block = &GETBLOCK(blockIDData[blockIndex - upDir - rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x2;
        }

        //Left front
        block = &GETBLOCK(blockIDData[blockIndex - rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x83;
        }

        //Top front
        block = &GETBLOCK(blockIDData[blockIndex + upDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0xE0;
        }

        //Right front
        block = &GETBLOCK(blockIDData[blockIndex + rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x38;
        }
    }

    result += vg::ConnectedTextureHelper::getOffsetFull(connectedOffset);
}

//Gets a grass side texture offset by looking at the surrounding blocks
void BlockTextureMethods::getGrassTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result) {
    const BlockPack* blocks = params.chunkMesher->blocks;
    int connectedOffset = 0;
    const int& blockIndex = params.chunkMesher->blockIndex;
    const int& upDir = params.upDir;
    const int& rightDir = params.rightDir;
    const int& frontDir = params.frontDir;
    const ui16* blockIDData = params.chunkMesher->blockData;
   
    const ChunkMesher* cm = params.chunkMesher;

    BlockTextureIndex tex = result;

    // Bottom Front
    const Block* block = &GETBLOCK(blockIDData[blockIndex - upDir + frontDir]);

    if (/*cm->levelOfDetail > 1 || */ TEXTURE_INDEX == tex) {
        block = &GETBLOCK(blockIDData[blockIndex]);
        result = block->textureTop->base.index;
        block->textureTop->base.blockTextureFunc(params, result);
        block->textureTop->base.getFinalColor(*params.color, cm->heightData->temperature, cm->heightData->rainfall, 0);
        return;
    }

    // Left
    block = &GETBLOCK(blockIDData[blockIndex - rightDir]);
    if (TEXTURE_INDEX == tex || block->occlude == BlockOcclusion::NONE) {
        connectedOffset |= 0x8;

        // REDUNDANT
        if (TEXTURE_INDEX == tex) {
            // bottom front Left
            block = &GETBLOCK(blockIDData[blockIndex - upDir - rightDir + frontDir]);
            if (TEXTURE_INDEX == tex) {
                connectedOffset |= 0xC;
            }
        }
    }

    // Front left
    block = &GETBLOCK(blockIDData[blockIndex - rightDir + frontDir]);
    if (TEXTURE_INDEX == tex) {
        connectedOffset |= 0x8;
    }

    // Bottom left
    block = &GETBLOCK(blockIDData[blockIndex - upDir - rightDir]);
    if (TEXTURE_INDEX == tex) {
        connectedOffset |= 0xC;
    }

    // bottom right
    block = &GETBLOCK(blockIDData[blockIndex - upDir + rightDir]);
    if (TEXTURE_INDEX == tex) {
        connectedOffset |= 0x3;
    }

    // Right
    block = &GETBLOCK(blockIDData[blockIndex + rightDir]);
    if (TEXTURE_INDEX == tex || block->occlude == BlockOcclusion::NONE) {
        connectedOffset |= 0x1;

        if (TEXTURE_INDEX == tex) {
            // bottom front Right
            block = &GETBLOCK(blockIDData[blockIndex - upDir + rightDir + frontDir]);
            if (TEXTURE_INDEX == tex) {
                connectedOffset |= 0x3;
            }
        }
    }

    // Front right
    block = &GETBLOCK(blockIDData[blockIndex + rightDir + frontDir]);
    if (TEXTURE_INDEX == tex) {
        connectedOffset |= 0x1;
    }

    result += vg::ConnectedTextureHelper::getOffsetSmall(connectedOffset);
}

void BlockTextureMethods::getVerticalTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result) {
    const BlockPack* blocks = params.chunkMesher->blocks;
    static int verticalOffsets[4] = { 0, 1, 3, 2 };

    int connectedOffset;
    const int& blockIndex = params.chunkMesher->blockIndex;
    const int& upDir = params.upDir;
    const ui16* blockIDData = params.chunkMesher->blockData;
    const ConnectedTextureReducedMethod& rm = params.blockTexInfo->reducedMethod;

    BlockTextureIndex tex = result;

    if (rm == ConnectedTextureReducedMethod::NONE) {
        connectedOffset = 0;
    } else if (rm == ConnectedTextureReducedMethod::TOP) {
        connectedOffset = 1;
    } else { //BOTTOM
        connectedOffset = 2;
    }

    //top bit
    const Block *block = &GETBLOCK(blockIDData[blockIndex + upDir]);

    if (TEXTURE_INDEX == tex) {
        connectedOffset |= 2;
    }
    //bottom bit
    block = &GETBLOCK(blockIDData[blockIndex - upDir]);
    if (TEXTURE_INDEX == tex) {
        connectedOffset |= 1;
    }

    result += verticalOffsets[connectedOffset];
}

void BlockTextureMethods::getHorizontalTextureIndex(BlockTextureMethodParams& params, BlockTextureIndex& result) {
    static int horizontalOffsets[4] = { 0, 1, 3, 2 };
    const BlockPack* blocks = params.chunkMesher->blocks;

    int connectedOffset = 0;
    const int& blockIndex = params.chunkMesher->blockIndex;
    const int& rightDir = params.rightDir;
    const int& frontDir = params.frontDir;
    const ui16* blockIDData = params.chunkMesher->blockData;
    BlockTextureIndex tex = result;

    //right bit
    const Block *block = &GETBLOCK(blockIDData[blockIndex + rightDir]);

    if (TEXTURE_INDEX == tex) {
        connectedOffset |= 1;
    }
    //left bit
    block = &GETBLOCK(blockIDData[blockIndex - rightDir]);
    if (TEXTURE_INDEX == tex) {
        connectedOffset |= 2;
    }

    if (params.blockTexInfo->innerSeams) {
        //front right bit
        block = &GETBLOCK(blockIDData[blockIndex + rightDir + frontDir]);
        if (TEXTURE_INDEX == tex) {
            connectedOffset &= 2;
        }
        //front left bit
        block = &GETBLOCK(blockIDData[blockIndex - rightDir + frontDir]);
        if (TEXTURE_INDEX == tex) {
            connectedOffset &= 1;
        }
    }

    result += horizontalOffsets[connectedOffset];
}
