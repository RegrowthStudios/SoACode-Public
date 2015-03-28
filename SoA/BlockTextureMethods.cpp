#include "stdafx.h"
#include "BlockTextureMethods.h"

#include <Vorb/graphics/ConnectedTextures.h>
#include <Vorb/utils.h>

#include "BlockPack.h"
#include "Chunk.h"
#include "ChunkMesh.h"
#include "RenderTask.h"
#include "VoxelBits.h"

//Gets a random offset for use by random textures
void BlockTextureMethods::getRandomTextureIndex(BlockTextureMethodParams& params, int& result) {
    //TODO: MurmurHash3
    const MesherInfo* mi = params.mesherInfo;
    const BlockTextureLayer* blockTexInfo = params.blockTexInfo;
    i32 seed = getPositionSeed(mi->x + mi->position.x, mi->y + mi->position.y, mi->z + mi->position.z);

    f32 r = (f32)((PseudoRand(seed) + 1.0) * 0.5 * blockTexInfo->totalWeight);
    f32 totalWeight = 0;

    // TODO(Ben): Binary search?
    if (blockTexInfo->weights.size()) {
        for (int i = 0; i < blockTexInfo->numTiles; i++) {
            totalWeight += blockTexInfo->weights[i];
            if (r <= totalWeight) {
                result += i;
                return;
            }
        }
    } else {
        for (int i = 0; i < blockTexInfo->numTiles; i++) {
            totalWeight += 1.0f;
            if (r <= totalWeight) {
                result += i;
                return;
            }
        }
    }
}

void BlockTextureMethods::getFloraTextureIndex(BlockTextureMethodParams& params, int& result) {
    //TODO: MurmurHash3
    const MesherInfo* mi = params.mesherInfo;
    i32 seed = getPositionSeed(mi->x + mi->position.x, mi->y + mi->position.y, mi->z + mi->position.z);

    f32 r = (f32)((PseudoRand(seed) + 1.0) * 0.5 * params.blockTexInfo->totalWeight);
    f32 totalWeight = 0;

    const BlockTextureLayer* blockTexInfo = params.blockTexInfo;
    const ui16* tertiaryData = mi->tertiaryData;

    const int& wc = mi->wc;

    int column;

    // TODO(Ben): Binary search?
    if (blockTexInfo->weights.size()) {
        for (int i = 0; i < blockTexInfo->size.x; i++) {
            totalWeight += blockTexInfo->weights[i];
            if (r <= totalWeight) {
                column = i;
                break;
            }
        }
    } else {
        for (int i = 0; i < blockTexInfo->size.x; i++) {
            totalWeight += 1.0f;
            if (r <= totalWeight) {
                column = i;
                break;
            }
        }
    }

    result += column;

    // Get the height of the current voxel
    int height = MIN(VoxelBits::getFloraHeight(tertiaryData[wc]), mi->currentBlock->floraHeight);
    int yPos = height - VoxelBits::getFloraPosition(tertiaryData[wc]);

    // Move the result to the flora of the correct height
    result += blockTexInfo->size.x * (height * height + height) / 2;
    // Offset by the ypos
    result += blockTexInfo->size.x * yPos;

}

//Gets a connected texture offset by looking at the surrounding blocks
void BlockTextureMethods::getConnectedTextureIndex(BlockTextureMethodParams& params, int& result) {

    int connectedOffset = 0;
    const int& wc = params.mesherInfo->wc;
    const int& upDir = params.upDir;
    const int& rightDir = params.rightDir;
    const int& frontDir = params.frontDir;
    const ui16* blockIDData = params.mesherInfo->blockIDData;
    const ui32& offset = params.offset;
    int tex = result;

    // Top Left
    Block *block = &GETBLOCK(blockIDData[wc + upDir - rightDir]);

    if (block->base[offset] != tex) {
        connectedOffset |= 0x80;
    }

    // Top
    block = &GETBLOCK(blockIDData[wc + upDir]);
    if (block->base[offset] != tex) {
        connectedOffset |= 0xE0;
    }

    // Top Right
    block = &GETBLOCK(blockIDData[wc + upDir + rightDir]);
    if (block->base[offset] != tex) {
        connectedOffset |= 0x20;
    }

    // Right
    block = &GETBLOCK(blockIDData[wc + rightDir]);
    if (block->base[offset] != tex) {
        connectedOffset |= 0x38;
    }

    // Bottom Right
    block = &GETBLOCK(blockIDData[wc - upDir + rightDir]);
    if (block->base[offset] != tex) {
        connectedOffset |= 0x8;
    }

    // Bottom
    block = &GETBLOCK(blockIDData[wc - upDir]);
    if (block->base[offset] != tex) {
        connectedOffset |= 0xE;
    }

    // Bottom Left
    block = &GETBLOCK(blockIDData[wc - upDir - rightDir]);
    if (block->base[offset] != tex) {
        connectedOffset |= 0x2;
    }

    // Left
    block = &GETBLOCK(blockIDData[wc - rightDir]);
    if (block->base[offset] != tex) {
        connectedOffset |= 0x83;
    }

    if (params.blockTexInfo->innerSeams) {
        // Top Front Left
        Block *block = &GETBLOCK(blockIDData[wc + upDir - rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x80;
        }

        // Top Front Right
        block = &GETBLOCK(blockIDData[wc + upDir + rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x20;
        }

        // Bottom front Right
        block = &GETBLOCK(blockIDData[wc - upDir + rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x8;
        }

        //Bottom front
        block = &GETBLOCK(blockIDData[wc - upDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0xE;
        }

        // Bottom front Left
        block = &GETBLOCK(blockIDData[wc - upDir - rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x2;
        }

        //Left front
        block = &GETBLOCK(blockIDData[wc - rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x83;
        }

        //Top front
        block = &GETBLOCK(blockIDData[wc + upDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0xE0;
        }

        //Right front
        block = &GETBLOCK(blockIDData[wc + rightDir + frontDir]);
        if (block->occlude != BlockOcclusion::NONE) {
            connectedOffset |= 0x38;
        }
    }

    result += vg::ConnectedTextureHelper::getOffsetFull(connectedOffset);
}

//Gets a grass side texture offset by looking at the surrounding blocks
void BlockTextureMethods::getGrassTextureIndex(BlockTextureMethodParams& params, int& result) {

    int connectedOffset = 0;
    const int& wc = params.mesherInfo->wc;
    const int& upDir = params.upDir;
    const int& rightDir = params.rightDir;
    const int& frontDir = params.frontDir;
    const ui16* blockIDData = params.mesherInfo->blockIDData;
    const ui32& offset = params.offset;
    const MesherInfo* mi = params.mesherInfo;
    
    int tex = result;

    // Bottom Front
    Block* block = &GETBLOCK(blockIDData[wc - upDir + frontDir]);
    if (mi->levelOfDetail > 1 || block->base[offset] == tex) {
        block = &GETBLOCK(blockIDData[wc]);
        result = block->pyTexInfo.base.textureIndex;
        block->pyTexInfo.base.blockTextureFunc(params, result);
        block->GetBlockColor(*params.color, 0, mi->temperature, mi->rainfall, block->pyTexInfo);
        return;
    }

    // Left
    block = &GETBLOCK(blockIDData[wc - rightDir]);
    if (block->base[offset] == tex || block->occlude == BlockOcclusion::NONE) {
        connectedOffset |= 0x8;

        if (block->base[offset] == tex) {
            // bottom front Left
            block = &GETBLOCK(blockIDData[wc - upDir - rightDir + frontDir]);
            if (block->base[offset] == tex) {
                connectedOffset |= 0xC;
            }
        }
    }

    // Front left
    block = &GETBLOCK(blockIDData[wc - rightDir + frontDir]);
    if (block->base[offset] == tex) {
        connectedOffset |= 0x8;
    }

    // Bottom left
    block = &GETBLOCK(blockIDData[wc - upDir - rightDir]);
    if (block->base[offset] == tex) {
        connectedOffset |= 0xC;
    }

    // bottom right
    block = &GETBLOCK(blockIDData[wc - upDir + rightDir]);
    if (block->base[offset] == tex) {
        connectedOffset |= 0x3;
    }

    // Right
    block = &GETBLOCK(blockIDData[wc + rightDir]);
    if (block->base[offset] == tex || block->occlude == BlockOcclusion::NONE) {
        connectedOffset |= 0x1;

        if (block->base[offset] == tex) {
            // bottom front Right
            block = &GETBLOCK(blockIDData[wc - upDir + rightDir + frontDir]);
            if (block->base[offset] == tex) {
                connectedOffset |= 0x3;
            }
        }
    }

    // Front right
    block = &GETBLOCK(blockIDData[wc + rightDir + frontDir]);
    if (block->base[offset] == tex) {
        connectedOffset |= 0x1;
    }

    result += vg::ConnectedTextureHelper::getOffsetSmall(connectedOffset);
}

void BlockTextureMethods::getVerticalTextureIndex(BlockTextureMethodParams& params, int& result) {

    static int verticalOffsets[4] = { 0, 1, 3, 2 };

    int connectedOffset;
    const int& wc = params.mesherInfo->wc;
    const int& upDir = params.upDir;
    const ui16* blockIDData = params.mesherInfo->blockIDData;
    const ConnectedTextureReducedMethod& rm = params.blockTexInfo->reducedMethod;

    int tex = result;

    if (rm == ConnectedTextureReducedMethod::NONE) {
        connectedOffset = 0;
    } else if (rm == ConnectedTextureReducedMethod::TOP) {
        connectedOffset = 1;
    } else { //BOTTOM
        connectedOffset = 2;
    }

    //top bit
    Block *block = &GETBLOCK(blockIDData[wc + upDir]);
    if (block->base[params.offset] == tex) {
        connectedOffset |= 2;
    }
    //bottom bit
    block = &GETBLOCK(blockIDData[wc - upDir]);
    if (block->base[params.offset] == tex) {
        connectedOffset |= 1;
    }

    result += verticalOffsets[connectedOffset];
}

void BlockTextureMethods::getHorizontalTextureIndex(BlockTextureMethodParams& params, int& result) {
    static int horizontalOffsets[4] = { 0, 1, 3, 2 };

    int connectedOffset = 0;
    const int& wc = params.mesherInfo->wc;
    const int& rightDir = params.rightDir;
    const int& frontDir = params.frontDir;
    const ui16* blockIDData = params.mesherInfo->blockIDData;
    int tex = result;

    //right bit
    Block *block = &GETBLOCK(blockIDData[wc + rightDir]);
    if (block->base[params.offset] == tex) {
        connectedOffset |= 1;
    }
    //left bit
    block = &GETBLOCK(blockIDData[wc - rightDir]);
    if (block->base[params.offset] == tex) {
        connectedOffset |= 2;
    }

    if (params.blockTexInfo->innerSeams) {
        //front right bit
        Block *block = &GETBLOCK(blockIDData[wc + rightDir + frontDir]);
        if (block->base[params.offset] == tex) {
            connectedOffset &= 2;
        }
        //front left bit
        block = &GETBLOCK(blockIDData[wc - rightDir + frontDir]);
        if (block->base[params.offset] == tex) {
            connectedOffset &= 1;
        }
    }

    result += horizontalOffsets[connectedOffset];
}
