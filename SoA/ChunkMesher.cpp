#include "stdafx.h"
#include "ChunkMesher.h"

#include <random>


#include "Biome.h"
#include "BlockData.h"

#include <Vorb/ThreadPool.h>
#include <Vorb/utils.h>

#include "BlockPack.h"
#include "Chunk.h"
#include "ChunkMeshTask.h"
#include "ChunkRenderer.h"
#include "Errors.h"
#include "GameManager.h"
#include "SoaOptions.h"
#include "VoxelBits.h"
#include "VoxelMesher.h"
#include "VoxelUtils.h"

#define GETBLOCK(a) blocks->operator[](a)

const float LIGHT_MULT = 0.95f, LIGHT_OFFSET = -0.2f;

const int MAXLIGHT = 31;

#define NO_QUAD_INDEX 0xFFFF

#define QUAD_SIZE 7

//#define USE_AO

// Base texture index
#define B_INDEX 0
// Overlay texture index
#define O_INDEX 1

const int X_NEG = (int)vvox::Cardinal::X_NEG;
const int X_POS = (int)vvox::Cardinal::X_POS;
const int Y_NEG = (int)vvox::Cardinal::Y_NEG;
const int Y_POS = (int)vvox::Cardinal::Y_POS;
const int Z_NEG = (int)vvox::Cardinal::Z_NEG;
const int Z_POS = (int)vvox::Cardinal::Z_POS;

#define UV_0 128
#define UV_1 129

// Meshing constants
//0 = x, 1 = y, 2 = z
const int FACE_AXIS[6][2] = { { 2, 1 }, { 2, 1 }, { 0, 2 }, { 0, 2 }, { 0, 1 }, { 0, 1 } };

const int FACE_AXIS_SIGN[6][2] = { { 1, 1 }, { -1, 1 }, { 1, 1 }, { -1, 1 }, { -1, 1 }, { 1, 1 } };

ChunkGridData ChunkMesher::defaultChunkGridData = {};

void ChunkMesher::init(const BlockPack* blocks) {
    this->blocks = blocks;

    // Set up the texture params
    m_textureMethodParams[X_NEG][B_INDEX].init(this, PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, -1, X_NEG, B_INDEX);
    m_textureMethodParams[X_NEG][O_INDEX].init(this, PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, -1, X_NEG, O_INDEX);

    m_textureMethodParams[X_POS][B_INDEX].init(this, -PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, 1, X_POS, B_INDEX);
    m_textureMethodParams[X_POS][O_INDEX].init(this, -PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, 1, X_POS, O_INDEX);

    m_textureMethodParams[Y_NEG][B_INDEX].init(this, -1, -PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, Y_NEG, B_INDEX);
    m_textureMethodParams[Y_NEG][O_INDEX].init(this, -1, -PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, Y_NEG, O_INDEX);

    m_textureMethodParams[Y_POS][B_INDEX].init(this, 1, -PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, Y_POS, B_INDEX);
    m_textureMethodParams[Y_POS][O_INDEX].init(this, 1, -PADDED_CHUNK_WIDTH, PADDED_CHUNK_LAYER, Y_POS, O_INDEX);

    m_textureMethodParams[Z_NEG][B_INDEX].init(this, -1, PADDED_CHUNK_LAYER, -PADDED_CHUNK_WIDTH, Z_NEG, B_INDEX);
    m_textureMethodParams[Z_NEG][O_INDEX].init(this, -1, PADDED_CHUNK_LAYER, -PADDED_CHUNK_WIDTH, Z_NEG, O_INDEX);

    m_textureMethodParams[Z_POS][B_INDEX].init(this, 1, PADDED_CHUNK_LAYER, PADDED_CHUNK_WIDTH, Z_POS, B_INDEX);
    m_textureMethodParams[Z_POS][O_INDEX].init(this, 1, PADDED_CHUNK_LAYER, PADDED_CHUNK_WIDTH, Z_POS, O_INDEX);
}

void ChunkMesher::prepareData(const Chunk* chunk) {
    int x, y, z, off1, off2;

    const Chunk* left = chunk->left;
    const Chunk* right = chunk->right;
    const Chunk* bottom = chunk->bottom;
    const Chunk* top = chunk->top;
    const Chunk* back = chunk->back;
    const Chunk* front = chunk->front;
    int wc;
    int c = 0;

    i32v3 pos;

    wSize = 0;
    chunk = chunk;
    m_chunkGridDataHandle = chunk->gridData;
    m_chunkGridData = m_chunkGridDataHandle.get();
    // Use default when none is provided
    if (!m_chunkGridData) m_chunkGridData = &defaultChunkGridData;

    // TODO(Ben): Do this last so we can be queued for mesh longer?
    // TODO(Ben): Dude macro this or something.
 
    if (chunk->blocks.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {

        int s = 0;
        //block data
        auto& dataTree = chunk->blocks.getTree();
        for (size_t i = 0; i < dataTree.size(); i++) {
            for (size_t j = 0; j < dataTree[i].length; j++) {
                c = dataTree[i].getStart() + j;

                getPosFromBlockIndex(c, pos);

                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);
                blockData[wc] = dataTree[i].data;
                if (GETBLOCK(blockData[wc]).meshType == MeshType::LIQUID) {
                    m_wvec[s++] = wc;
                }

            }
        }
        wSize = s;
    } else {
        int s = 0;
        for (y = 0; y < CHUNK_WIDTH; y++) {
            for (z = 0; z < CHUNK_WIDTH; z++) {
                for (x = 0; x < CHUNK_WIDTH; x++, c++) {
                    wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                    blockData[wc] = chunk->blocks[c];
                    if (GETBLOCK(blockData[wc]).meshType == MeshType::LIQUID) {
                        m_wvec[s++] = wc;
                    }
                }
            }
        }
        wSize = s;
    }
    if (chunk->tertiary.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
        //tertiary data
        c = 0;
        auto& dataTree = chunk->tertiary.getTree();
        for (size_t i = 0; i < dataTree.size(); i++) {
            for (size_t j = 0; j < dataTree[i].length; j++) {
                c = dataTree[i].getStart() + j;

                getPosFromBlockIndex(c, pos);
                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);

                tertiaryData[wc] = dataTree[i].data;
            }
        }

    } else {
        c = 0;
        for (y = 0; y < CHUNK_WIDTH; y++) {
            for (z = 0; z < CHUNK_WIDTH; z++) {
                for (x = 0; x < CHUNK_WIDTH; x++, c++) {
                    wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                    tertiaryData[wc] = chunk->tertiary.get(c);
                }
            }
        }
    }

    if (left) {
        for (y = 1; y < PADDED_WIDTH - 1; y++) {
            for (z = 1; z < PADDED_WIDTH - 1; z++) {
                off1 = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
                off2 = z*PADDED_WIDTH + y*PADDED_LAYER;

                blockData[off2] = left->getBlockData(off1 + CHUNK_WIDTH - 1);
                tertiaryData[off2] = left->getTertiaryData(off1 + CHUNK_WIDTH - 1);
            }
        }
    }

    if (right) {
        for (y = 1; y < PADDED_WIDTH - 1; y++) {
            for (z = 1; z < PADDED_WIDTH - 1; z++) {
                off1 = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
                off2 = z*PADDED_WIDTH + y*PADDED_LAYER;

                blockData[off2 + PADDED_WIDTH - 1] = (right->getBlockData(off1));
                tertiaryData[off2 + PADDED_WIDTH - 1] = right->getTertiaryData(off1);
            }
        }
    }

    if (bottom) {
        for (z = 1; z < PADDED_WIDTH - 1; z++) {
            for (x = 1; x < PADDED_WIDTH - 1; x++) {
                off1 = (z - 1)*CHUNK_WIDTH + x - 1;
                off2 = z*PADDED_WIDTH + x;
                //data
                blockData[off2] = (bottom->getBlockData(CHUNK_SIZE - CHUNK_LAYER + off1)); //bottom
                tertiaryData[off2] = bottom->getTertiaryData(CHUNK_SIZE - CHUNK_LAYER + off1);
            }
        }
    }

    if (top) {
        for (z = 1; z < PADDED_WIDTH - 1; z++) {
            for (x = 1; x < PADDED_WIDTH - 1; x++) {
                off1 = (z - 1)*CHUNK_WIDTH + x - 1;
                off2 = z*PADDED_WIDTH + x;

                blockData[off2 + PADDED_SIZE - PADDED_LAYER] = (top->getBlockData(off1)); //top
                tertiaryData[off2 + PADDED_SIZE - PADDED_LAYER] = top->getTertiaryData(off1);
            }
        }
    }

    if (back) {
        for (y = 1; y < PADDED_WIDTH - 1; y++) {
            for (x = 1; x < PADDED_WIDTH - 1; x++) {
                off1 = (x - 1) + (y - 1)*CHUNK_LAYER;
                off2 = x + y*PADDED_LAYER;

                blockData[off2] = back->getBlockData(off1 + CHUNK_LAYER - CHUNK_WIDTH);
                tertiaryData[off2] = back->getTertiaryData(off1 + CHUNK_LAYER - CHUNK_WIDTH);
            }
        }
    }

    if (front) {
        for (y = 1; y < PADDED_WIDTH - 1; y++) {
            for (x = 1; x < PADDED_WIDTH - 1; x++) {
                off1 = (x - 1) + (y - 1)*CHUNK_LAYER;
                off2 = x + y*PADDED_LAYER;

                blockData[off2 + PADDED_LAYER - PADDED_WIDTH] = (front->getBlockData(off1));
                tertiaryData[off2 + PADDED_LAYER - PADDED_WIDTH] = front->getTertiaryData(off1);
            }
        }
    }
}

#define GET_EDGE_X(ch, sy, sz, dy, dz) \
    ch->lock(); \
    for (int x = 0; x < CHUNK_WIDTH; x++) { \
        srcIndex = (sy) * CHUNK_LAYER + (sz) * CHUNK_WIDTH + x; \
        destIndex = (dy) * PADDED_LAYER + (dz) * PADDED_WIDTH + (x + 1); \
        blockData[destIndex] = ch->getBlockData(srcIndex); \
        tertiaryData[destIndex] = ch->getTertiaryData(srcIndex); \
                } \
    ch->unlock();

#define GET_EDGE_Y(ch, sx, sz, dx, dz) \
    ch->lock(); \
    for (int y = 0; y < CHUNK_WIDTH; y++) { \
        srcIndex = y * CHUNK_LAYER + (sz) * CHUNK_WIDTH + (sx); \
        destIndex = (y + 1) * PADDED_LAYER + (dz) * PADDED_WIDTH + (dx); \
        blockData[destIndex] = ch->getBlockData(srcIndex); \
        tertiaryData[destIndex] = ch->getTertiaryData(srcIndex); \
            } \
    ch->unlock();

#define GET_EDGE_Z(ch, sx, sy, dx, dy) \
    ch->lock(); \
    for (int z = 0; z < CHUNK_WIDTH; z++) { \
        srcIndex = z * CHUNK_WIDTH + (sy) * CHUNK_LAYER + (sx); \
        destIndex = (z + 1) * PADDED_WIDTH + (dy) * PADDED_LAYER + (dx); \
        blockData[destIndex] = ch->getBlockData(srcIndex); \
        tertiaryData[destIndex] = ch->getTertiaryData(srcIndex); \
        } \
    ch->unlock();

#define GET_CORNER(ch, sx, sy, sz, dx, dy, dz) \
    srcIndex = (sy) * CHUNK_LAYER + (sz) * CHUNK_WIDTH + (sx); \
    destIndex = (dy) * PADDED_LAYER + (dz) * PADDED_WIDTH + (dx); \
    ch->lock(); \
    blockData[destIndex] = ch->getBlockData(srcIndex); \
    tertiaryData[destIndex] = ch->getTertiaryData(srcIndex); \
    ch->unlock();

void ChunkMesher::prepareDataAsync(Chunk* chunk) {
    int x, y, z, srcIndex, destIndex;

    Chunk* left = chunk->left;
    Chunk* right = chunk->right;
    Chunk* bottom = chunk->bottom;
    Chunk* top = chunk->top;
    Chunk* back = chunk->back;
    Chunk* front = chunk->front;
    int wc;
    int c = 0;

    i32v3 pos;

    memset(blockData, 0, sizeof(blockData));
    memset(tertiaryData, 0, sizeof(tertiaryData));

    wSize = 0;
    chunk = chunk;
    m_chunkGridDataHandle = chunk->gridData;
    m_chunkGridData = m_chunkGridDataHandle.get();
    // Use default when none is provided
    if (!m_chunkGridData) m_chunkGridData = &defaultChunkGridData;

    // TODO(Ben): Do this last so we can be queued for mesh longer?
    // TODO(Ben): Dude macro this or something.
    chunk->lock();
    chunk->queuedForMesh = false; ///< Indicate that we are no longer queued for a mesh
    if (chunk->blocks.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {

        int s = 0;
        //block data
        auto& dataTree = chunk->blocks.getTree();
        for (size_t i = 0; i < dataTree.size(); i++) {
            for (size_t j = 0; j < dataTree[i].length; j++) {
                c = dataTree[i].getStart() + j;

                getPosFromBlockIndex(c, pos);

                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);
                blockData[wc] = dataTree[i].data;
                if (GETBLOCK(blockData[wc]).meshType == MeshType::LIQUID) {
                    m_wvec[s++] = wc;
                }

            }
        }
        wSize = s;
    } else {
        int s = 0;
        for (y = 0; y < CHUNK_WIDTH; y++) {
            for (z = 0; z < CHUNK_WIDTH; z++) {
                for (x = 0; x < CHUNK_WIDTH; x++, c++) {
                    wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                    blockData[wc] = chunk->blocks[c];
                    if (GETBLOCK(blockData[wc]).meshType == MeshType::LIQUID) {
                        m_wvec[s++] = wc;
                    }
                }
            }
        }
        wSize = s;
    }
    if (chunk->tertiary.getState() == vvox::VoxelStorageState::INTERVAL_TREE) {
        //tertiary data
        c = 0;
        auto& dataTree = chunk->tertiary.getTree();
        for (size_t i = 0; i < dataTree.size(); i++) {
            for (size_t j = 0; j < dataTree[i].length; j++) {
                c = dataTree[i].getStart() + j;

                getPosFromBlockIndex(c, pos);
                wc = (pos.y + 1)*PADDED_LAYER + (pos.z + 1)*PADDED_WIDTH + (pos.x + 1);

                tertiaryData[wc] = dataTree[i].data;
            }
        }

    } else {
        c = 0;
        for (y = 0; y < CHUNK_WIDTH; y++) {
            for (z = 0; z < CHUNK_WIDTH; z++) {
                for (x = 0; x < CHUNK_WIDTH; x++, c++) {
                    wc = (y + 1)*PADDED_LAYER + (z + 1)*PADDED_WIDTH + (x + 1);
                    tertiaryData[wc] = chunk->tertiary.get(c);
                }
            }
        }
    }
    chunk->unlock();

    left->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        for (z = 1; z < PADDED_WIDTH - 1; z++) {
            srcIndex = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
            destIndex = z*PADDED_WIDTH + y*PADDED_LAYER;

            blockData[destIndex] = left->getBlockData(srcIndex + CHUNK_WIDTH - 1);
            tertiaryData[destIndex] = left->getTertiaryData(srcIndex + CHUNK_WIDTH - 1);
        }
    }
    left->unlock();

    right->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        for (z = 1; z < PADDED_WIDTH - 1; z++) {
            srcIndex = (z - 1)*CHUNK_WIDTH + (y - 1)*CHUNK_LAYER;
            destIndex = z*PADDED_WIDTH + y*PADDED_LAYER + PADDED_WIDTH - 1;

            blockData[destIndex] = (right->getBlockData(srcIndex));
            tertiaryData[destIndex] = right->getTertiaryData(srcIndex);
        }
    }
    right->unlock();

    bottom->lock();
    for (z = 1; z < PADDED_WIDTH - 1; z++) {
        for (x = 1; x < PADDED_WIDTH - 1; x++) {
            srcIndex = (z - 1)*CHUNK_WIDTH + x - 1 + CHUNK_SIZE - CHUNK_LAYER;
            destIndex = z*PADDED_WIDTH + x;
            //data
            blockData[destIndex] = (bottom->getBlockData(srcIndex)); //bottom
            tertiaryData[destIndex] = bottom->getTertiaryData(srcIndex);
        }
    }
    bottom->unlock();

    top->lock();
    for (z = 1; z < PADDED_WIDTH - 1; z++) {
        for (x = 1; x < PADDED_WIDTH - 1; x++) {
            srcIndex = (z - 1)*CHUNK_WIDTH + x - 1;
            destIndex = z*PADDED_WIDTH + x + PADDED_SIZE - PADDED_LAYER;

            blockData[destIndex] = (top->getBlockData(srcIndex)); //top
            tertiaryData[destIndex] = top->getTertiaryData(srcIndex);
        }
    }
    top->unlock();

    back->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        for (x = 1; x < PADDED_WIDTH - 1; x++) {
            srcIndex = (x - 1) + (y - 1)*CHUNK_LAYER + CHUNK_LAYER - CHUNK_WIDTH;
            destIndex = x + y*PADDED_LAYER;

            blockData[destIndex] = back->getBlockData(srcIndex);
            tertiaryData[destIndex] = back->getTertiaryData(srcIndex);
        }
    }
    back->unlock();

    front->lock();
    for (y = 1; y < PADDED_WIDTH - 1; y++) {
        for (x = 1; x < PADDED_WIDTH - 1; x++) {
            srcIndex = (x - 1) + (y - 1)*CHUNK_LAYER;
            destIndex = x + y*PADDED_LAYER + PADDED_LAYER - PADDED_WIDTH;

            blockData[destIndex] = front->getBlockData(srcIndex);
            tertiaryData[destIndex] = front->getTertiaryData(srcIndex);
        }
    }
    front->unlock();

    // Top Back
    Chunk* topBack = top->back;
    GET_EDGE_X(topBack, 0, CHUNK_WIDTH - 1, PADDED_WIDTH - 1, 0);
    // Top Front
    Chunk* topFront = top->front;
    GET_EDGE_X(topFront, 0, 0, PADDED_WIDTH - 1, PADDED_WIDTH - 1);

    // Bottom Back
    Chunk* bottomBack = bottom->back;
    GET_EDGE_X(bottomBack, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1, 0, 0);
    // Bottom Front
    Chunk* bottomFront = bottom->front;
    GET_EDGE_X(bottomFront, CHUNK_WIDTH - 1, 0, 0, PADDED_WIDTH - 1);

    // Left half
    int srcXOff = CHUNK_WIDTH - 1;
    int dstXOff = 0;
    Chunk* leftBack = left->back;
    GET_EDGE_Y(leftBack, srcXOff, CHUNK_WIDTH - 1, dstXOff, 0);
    Chunk* leftFront = left->front;
    GET_EDGE_Y(leftFront, srcXOff, 0, dstXOff, PADDED_WIDTH - 1);

    Chunk* leftTop = left->top;
    GET_EDGE_Z(leftTop, CHUNK_WIDTH - 1, 0, 0, PADDED_WIDTH - 1);
    Chunk* leftBot = left->bottom;
    GET_EDGE_Z(leftBot, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1, 0, 0);

    // Right half
    srcXOff = 0;
    dstXOff = PADDED_WIDTH - 1;
    Chunk* rightBack = right->back;
    GET_EDGE_Y(rightBack, srcXOff, CHUNK_WIDTH - 1, dstXOff, 0);
    Chunk* rightFront = right->front;
    GET_EDGE_Y(rightFront, srcXOff, 0, dstXOff, PADDED_WIDTH - 1);

    Chunk* rightTop = right->top;
    GET_EDGE_Z(rightTop, srcXOff, 0, dstXOff, PADDED_WIDTH - 1);
    Chunk* rightBot = right->bottom;
    GET_EDGE_Z(rightBot, srcXOff, CHUNK_WIDTH - 1, dstXOff, 0);

    // 8 Corners
    // Left Corners
    Chunk* leftTopBack = leftTop->back;
    GET_CORNER(leftTopBack, CHUNK_WIDTH - 1, 0, CHUNK_WIDTH - 1,
               0, PADDED_WIDTH - 1, 0);
    Chunk* leftTopFront = leftTop->front;
    GET_CORNER(leftTopFront, CHUNK_WIDTH - 1, 0, 0,
               0, PADDED_WIDTH - 1, PADDED_WIDTH - 1);
    Chunk* leftBottomBack = leftBot->back;
    GET_CORNER(leftBottomBack, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1,
               0, 0, 0);
    Chunk* leftBottomFront = leftBot->front;
    GET_CORNER(leftBottomFront, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1, 0,
               0, 0, PADDED_WIDTH - 1);
    // Right Corners
    Chunk* rightTopBack = rightTop->back;
    GET_CORNER(rightTopBack, 0, 0, CHUNK_WIDTH - 1,
               PADDED_WIDTH - 1, PADDED_WIDTH - 1, 0);
    Chunk* rightTopFront = rightTop->front;
    GET_CORNER(rightTopFront, 0, 0, 0,
               PADDED_WIDTH - 1, PADDED_WIDTH - 1, PADDED_WIDTH - 1);
    Chunk* rightBottomBack = rightBot->back;
    GET_CORNER(rightBottomBack, 0, CHUNK_WIDTH - 1, CHUNK_WIDTH - 1,
               PADDED_WIDTH - 1, 0, 0);
    Chunk* rightBottomFront = rightBot->front;
    GET_CORNER(rightBottomFront, 0, CHUNK_WIDTH - 1, 0,
               PADDED_WIDTH - 1, 0, PADDED_WIDTH - 1);
}

CALLER_DELETE ChunkMeshData* ChunkMesher::createChunkMeshData(MeshTaskType type) {
    m_numQuads = 0;
    m_highestY = 0;
    m_lowestY = 256;
    m_highestX = 0;
    m_lowestX = 256;
    m_highestZ = 0;
    m_lowestZ = 256;

    // Clear quad indices
    memset(m_quadIndices, 0xFF, sizeof(m_quadIndices));

    for (int i = 0; i < 6; i++) {
        m_quads[i].clear();
    }

    // TODO(Ben): Here?
    _waterVboVerts.clear();
    _cutoutVerts.clear();

    // Stores the data for a chunk mesh
    // TODO(Ben): new is bad mkay
    m_chunkMeshData = new ChunkMeshData(MeshTaskType::DEFAULT);

    // Loop through blocks
    for (by = 0; by < CHUNK_WIDTH; by++) {
        for (bz = 0; bz < CHUNK_WIDTH; bz++) {
            for (bx = 0; bx < CHUNK_WIDTH; bx++) {
                // Get data for this voxel
                // TODO(Ben): Could optimize out -1
                blockIndex = (by + 1) * PADDED_CHUNK_LAYER + (bz + 1) * PADDED_CHUNK_WIDTH + (bx + 1);
                blockID = blockData[blockIndex];
                if (blockID == 0) continue; // Skip air blocks
                heightData = &m_chunkGridData->heightData[bz * CHUNK_WIDTH + bx];
                block = &blocks->operator[](blockID);
                // TODO(Ben) Don't think bx needs to be member
                voxelPosOffset = ui8v3(bx * QUAD_SIZE, by * QUAD_SIZE, bz * QUAD_SIZE);

                switch (block->meshType) {
                    case MeshType::BLOCK:
                        addBlock();
                        break;
                    case MeshType::LEAVES:
                    case MeshType::CROSSFLORA:
                    case MeshType::FLORA:
                        addFlora();
                        break;
                    default:
                        //No mesh, do nothing
                        break;
                }
            }
        }
    }

    ChunkMeshRenderData& renderData = m_chunkMeshData->chunkMeshRenderData;

    // Get quad buffer to fill
    std::vector<VoxelQuad>& finalQuads = m_chunkMeshData->opaqueQuads;

    finalQuads.resize(m_numQuads);
    // Copy the data
    // TODO(Ben): Could construct in place and not need ANY copying with 6 iterations?
    i32 index = 0;
    i32 sizes[6];
    for (int i = 0; i < 6; i++) {
        std::vector<VoxelQuad>& quads = m_quads[i];
        int tmp = index;
        for (size_t j = 0; j < quads.size(); j++) {
            VoxelQuad& q = quads[j];
            if (q.v0.mesherFlags & MESH_FLAG_ACTIVE) {
                finalQuads[index++] = q;
            }
        }
        sizes[i] = index - tmp;
    }

    m_highestY /= QUAD_SIZE;
    m_lowestY /= QUAD_SIZE;
    m_highestX /= QUAD_SIZE;
    m_lowestX /= QUAD_SIZE;
    m_highestZ /= QUAD_SIZE;
    m_lowestZ /= QUAD_SIZE;

#define INDICES_PER_QUAD 6

    if (finalQuads.size()) {
        renderData.nxVboOff = 0;
        renderData.nxVboSize = sizes[0] * INDICES_PER_QUAD;
        renderData.pxVboOff = renderData.nxVboSize;
        renderData.pxVboSize = sizes[1] * INDICES_PER_QUAD;
        renderData.nyVboOff = renderData.pxVboOff + renderData.pxVboSize;
        renderData.nyVboSize = sizes[2] * INDICES_PER_QUAD;
        renderData.pyVboOff = renderData.nyVboOff + renderData.nyVboSize;
        renderData.pyVboSize = sizes[3] * INDICES_PER_QUAD;
        renderData.nzVboOff = renderData.pyVboOff + renderData.pyVboSize;
        renderData.nzVboSize = sizes[4] * INDICES_PER_QUAD;
        renderData.pzVboOff = renderData.nzVboOff + renderData.nzVboSize;
        renderData.pzVboSize = sizes[5] * INDICES_PER_QUAD;
        renderData.indexSize = finalQuads.size() * INDICES_PER_QUAD;

        // Redundant
        renderData.highestX = m_highestX;
        renderData.lowestX = m_lowestX;
        renderData.highestY = m_highestY;
        renderData.lowestY = m_lowestY;
        renderData.highestZ = m_highestZ;
        renderData.lowestZ = m_lowestZ;
    }

    m_chunkGridDataHandle.reset();

    return m_chunkMeshData;
}

inline bool mapBufferData(GLuint& vboID, GLsizeiptr size, void* src, GLenum usage) {
    // Block Vertices
    if (vboID == 0) {
        glGenBuffers(1, &(vboID)); // Create the buffer ID
    }
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, usage);

    void *v = glMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

    if (v == NULL) return false;

    memcpy(v, src, size);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

bool ChunkMesher::uploadMeshData(ChunkMesh& mesh, ChunkMeshData* meshData) {
    bool canRender = false;

    //store the index data for sorting in the chunk mesh
    mesh.transQuadIndices.swap(meshData->transQuadIndices);
    mesh.transQuadPositions.swap(meshData->transQuadPositions);

    switch (meshData->type) {
        case MeshTaskType::DEFAULT:
            if (meshData->opaqueQuads.size()) {

                mapBufferData(mesh.vboID, meshData->opaqueQuads.size() * sizeof(VoxelQuad), &(meshData->opaqueQuads[0]), GL_STATIC_DRAW);
                canRender = true;

                if (!mesh.vaoID) buildVao(mesh);
            } else {
                if (mesh.vboID != 0) {
                    glDeleteBuffers(1, &(mesh.vboID));
                    mesh.vboID = 0;
                }
                if (mesh.vaoID != 0) {
                    glDeleteVertexArrays(1, &(mesh.vaoID));
                    mesh.vaoID = 0;
                }
            }

            if (meshData->transQuads.size()) {

                //vertex data
                mapBufferData(mesh.transVboID, meshData->transQuads.size() * sizeof(VoxelQuad), &(meshData->transQuads[0]), GL_STATIC_DRAW);

                //index data
                mapBufferData(mesh.transIndexID, mesh.transQuadIndices.size() * sizeof(ui32), &(mesh.transQuadIndices[0]), GL_STATIC_DRAW);
                canRender = true;
                mesh.needsSort = true; //must sort when changing the mesh

                if (!mesh.transVaoID) buildTransparentVao(mesh);
            } else {
                if (mesh.transVaoID != 0) {
                    glDeleteVertexArrays(1, &(mesh.transVaoID));
                    mesh.transVaoID = 0;
                }
                if (mesh.transVboID != 0) {
                    glDeleteBuffers(1, &(mesh.transVboID));
                    mesh.transVboID = 0;
                }
                if (mesh.transIndexID != 0) {
                    glDeleteBuffers(1, &(mesh.transIndexID));
                    mesh.transIndexID = 0;
                }
            }

            if (meshData->cutoutQuads.size()) {

                mapBufferData(mesh.cutoutVboID, meshData->cutoutQuads.size() * sizeof(VoxelQuad), &(meshData->cutoutQuads[0]), GL_STATIC_DRAW);
                canRender = true;
                if (!mesh.cutoutVaoID) buildCutoutVao(mesh);
            } else {
                if (mesh.cutoutVaoID != 0) {
                    glDeleteVertexArrays(1, &(mesh.cutoutVaoID));
                    mesh.cutoutVaoID = 0;
                }
                if (mesh.cutoutVboID != 0) {
                    glDeleteBuffers(1, &(mesh.cutoutVboID));
                    mesh.cutoutVboID = 0;
                }
            }
            mesh.renderData = meshData->chunkMeshRenderData;
            //The missing break is deliberate!
        case MeshTaskType::LIQUID:

            mesh.renderData.waterIndexSize = meshData->chunkMeshRenderData.waterIndexSize;
            if (meshData->waterVertices.size()) {
                mapBufferData(mesh.waterVboID, meshData->waterVertices.size() * sizeof(LiquidVertex), &(meshData->waterVertices[0]), GL_STREAM_DRAW);
                canRender = true;
                if (!mesh.waterVaoID) buildWaterVao(mesh);
            } else {
                if (mesh.waterVboID != 0) {
                    glDeleteBuffers(1, &(mesh.waterVboID));
                    mesh.waterVboID = 0;
                }
                if (mesh.waterVaoID != 0) {
                    glDeleteVertexArrays(1, &(mesh.waterVaoID));
                    mesh.waterVaoID = 0;
                }
            }
            break;
    }
    return canRender;
}

void ChunkMesher::freeChunkMesh(CALLER_DELETE ChunkMesh* mesh) {
    // Opaque
    if (mesh->vboID != 0) {
        glDeleteBuffers(1, &(mesh->vboID));
        mesh->vboID = 0;
    }
    if (mesh->vaoID != 0) {
        glDeleteVertexArrays(1, &(mesh->vaoID));
        mesh->vaoID = 0;
    }
    // Transparent
    if (mesh->transVaoID != 0) {
        glDeleteVertexArrays(1, &(mesh->transVaoID));
        mesh->transVaoID = 0;
    }
    if (mesh->transVboID != 0) {
        glDeleteBuffers(1, &(mesh->transVboID));
        mesh->transVboID = 0;
    }
    if (mesh->transIndexID != 0) {
        glDeleteBuffers(1, &(mesh->transIndexID));
        mesh->transIndexID = 0;
    }
    // Cutout
    if (mesh->cutoutVaoID != 0) {
        glDeleteVertexArrays(1, &(mesh->cutoutVaoID));
        mesh->cutoutVaoID = 0;
    }
    if (mesh->cutoutVboID != 0) {
        glDeleteBuffers(1, &(mesh->cutoutVboID));
        mesh->cutoutVboID = 0;
    }
    // Liquid
    if (mesh->waterVboID != 0) {
        glDeleteBuffers(1, &(mesh->waterVboID));
        mesh->waterVboID = 0;
    }
    if (mesh->waterVaoID != 0) {
        glDeleteVertexArrays(1, &(mesh->waterVaoID));
        mesh->waterVaoID = 0;
    }
    delete mesh;
}
//
//CALLEE_DELETE ChunkMeshData* ChunkMesher::createOnlyWaterMesh(const Chunk* chunk) {
//    /*if (chunkMeshData != NULL) {
//        pError("Tried to create mesh with in use chunkMeshData!");
//        return 0;
//        }
//        chunkMeshData = new ChunkMeshData(renderTask);
//
//        _waterVboVerts.clear();
//
//        mi.task = renderTask;
//
//        for (int i = 0; i < wSize; i++) {
//        mi.wc = m_wvec[i];
//        mi.btype = GETBLOCKID(blockData[mi.wc]);
//        mi.x = (mi.wc % PADDED_CHUNK_WIDTH) - 1;
//        mi.y = (mi.wc / PADDED_CHUNK_LAYER) - 1;
//        mi.z = ((mi.wc % PADDED_CHUNK_LAYER) / PADDED_CHUNK_WIDTH) - 1;
//
//        addLiquid(mi);
//        }
//
//
//        if (mi.liquidIndex) {
//        chunkMeshData->chunkMeshRenderData.waterIndexSize = (mi.liquidIndex * 6) / 4;
//        chunkMeshData->waterVertices.swap(_waterVboVerts);
//        }*/
//
//    return nullptr;
//}

void ChunkMesher::freeBuffers() {
    //free memory
    //std::vector <BlockVertex>().swap(_vboVerts);

    //These dont get too big so it might be ok to not free them?
    /*vector <Vertex>().swap(waterVboVerts);
    vector<Vertex>().swap(finalTopVerts);
    vector<Vertex>().swap(finalLeftVerts);
    vector<Vertex>().swap(finalRightVerts);
    vector<Vertex>().swap(finalFrontVerts);
    vector<Vertex>().swap(finalBackVerts);
    vector<Vertex>().swap(finalBottomVerts);
    vector<Vertex>().swap(finalNbVerts);*/
}

#define CompareVertices(v1, v2) (!memcmp(&v1.color, &v2.color, 3) && v1.sunlight == v2.sunlight && !memcmp(&v1.lampColor, &v2.lampColor, 3)  \
    && !memcmp(&v1.overlayColor, &v2.overlayColor, 3) \
    && v1.textureAtlas == v2.textureAtlas && v1.textureIndex == v2.textureIndex && v1.overlayTextureAtlas == v2.overlayTextureAtlas && v1.overlayTextureIndex == v2.overlayTextureIndex)

#define CompareVerticesLight(v1, v2) (v1.sunlight == v2.sunlight && !memcmp(&v1.lampColor, &v2.lampColor, 3) && !memcmp(&v1.color, &v2.color, 3))

void ChunkMesher::addBlock()
{
    // Ambient occlusion buffer for vertices
    f32 ao[4];

    // Check the faces
    // Left
    if (shouldRenderFace(-1)) {
        computeAmbientOcclusion(-1, -PADDED_CHUNK_LAYER, PADDED_CHUNK_WIDTH, ao);
        addQuad(X_NEG, (int)vvox::Axis::Z, (int)vvox::Axis::Y, -PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, 2, ui8v2(1, 1), ao);
    }
    // Right
    if (shouldRenderFace(1)) {
        computeAmbientOcclusion(1, -PADDED_CHUNK_LAYER, -PADDED_CHUNK_WIDTH, ao);
        addQuad(X_POS, (int)vvox::Axis::Z, (int)vvox::Axis::Y, -PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, 0, ui8v2(-1, 1), ao);
    }
    // Bottom
    if (shouldRenderFace(-PADDED_CHUNK_LAYER)) { 
        computeAmbientOcclusion(-PADDED_CHUNK_LAYER, PADDED_CHUNK_WIDTH, 1, ao);
        addQuad(Y_NEG, (int)vvox::Axis::X, (int)vvox::Axis::Z, -1, -PADDED_CHUNK_WIDTH, 2, ui8v2(1, 1), ao);
    }
    // Top
    if (shouldRenderFace(PADDED_CHUNK_LAYER)) {
        computeAmbientOcclusion(PADDED_CHUNK_LAYER, -PADDED_CHUNK_WIDTH, -1, ao);
        addQuad(Y_POS, (int)vvox::Axis::X, (int)vvox::Axis::Z, -1, -PADDED_CHUNK_WIDTH, 0, ui8v2(-1, 1), ao);
    }
    // Back
    if (shouldRenderFace(-PADDED_CHUNK_WIDTH)) {
        computeAmbientOcclusion(-PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, -1, ao);
        addQuad(Z_NEG, (int)vvox::Axis::X, (int)vvox::Axis::Y, -1, -PADDED_CHUNK_LAYER, 0, ui8v2(-1, 1), ao);
    }
    // Front
    if (shouldRenderFace(PADDED_CHUNK_WIDTH)) {
        computeAmbientOcclusion(PADDED_CHUNK_WIDTH, -PADDED_CHUNK_LAYER, 1, ao);
        addQuad(Z_POS, (int)vvox::Axis::X, (int)vvox::Axis::Y, -1, -PADDED_CHUNK_LAYER, 2, ui8v2(1, 1), ao);
    }
}

void ChunkMesher::computeAmbientOcclusion(int upOffset, int frontOffset, int rightOffset, f32 ambientOcclusion[]) {
#ifdef USE_AO
    // Ambient occlusion factor
#define OCCLUSION_FACTOR 0.2f;
    // Helper macro
    // TODO(Ben): This isn't exactly right since self will occlude. Use a function
#define CALCULATE_VERTEX(v, s1, s2) \
    nearOccluders = getOcclusion(blocks->operator[](blockData[blockIndex])) + \
    getOcclusion(blocks->operator[](blockData[blockIndex s1 frontOffset])) + \
    getOcclusion(blocks->operator[](blockData[blockIndex s2 rightOffset])) + \
    getOcclusion(blocks->operator[](blockData[blockIndex s1 frontOffset s2 rightOffset])); \
    ambientOcclusion[v] = 1.0f - nearOccluders * OCCLUSION_FACTOR; 
   
    // Move the block index upwards
    int blockIndex = this->blockIndex + upOffset;
    int nearOccluders; ///< For ambient occlusion

    // TODO(Ben): I know for a fact the inputs are wrong for some faces

    // Vertex 0
    CALCULATE_VERTEX(0, -, -)

    // Vertex 1
    CALCULATE_VERTEX(1, +, -)

    // Vertex 2
    CALCULATE_VERTEX(2, +, +)

    // Vertex 3
    CALCULATE_VERTEX(3, -, +)
#endif
}

void ChunkMesher::addQuad(int face, int rightAxis, int frontAxis, int leftOffset, int backOffset, int rightStretchIndex, const ui8v2& texOffset, f32 ambientOcclusion[]) {
    // Get texture TODO(Ben): Null check?
    const BlockTexture* texture = block->textures[face];
    // Get colors
    // TODO(Ben): altColors
    color3 blockColor[2];
    texture->base.getFinalColor(blockColor[B_INDEX],
                                heightData->temperature,
                                heightData->rainfall, 0);
    texture->base.getFinalColor(blockColor[O_INDEX],
                                heightData->temperature,
                                heightData->rainfall, 0);

    std::vector<VoxelQuad>& quads = m_quads[face];

    // Get texturing parameters
    ui8 blendMode = getBlendMode(texture->blendMode);
    // TODO(Ben): Make this better
    BlockTextureIndex baseTextureIndex = texture->base.getBlockTextureIndex(m_textureMethodParams[face][B_INDEX], blockColor[B_INDEX]);
    BlockTextureIndex baseNormTextureIndex = texture->base.getNormalTextureIndex(m_textureMethodParams[face][B_INDEX], blockColor[B_INDEX]);
    BlockTextureIndex baseDispTextureIndex = texture->base.getDispTextureIndex(m_textureMethodParams[face][B_INDEX], blockColor[B_INDEX]);
    BlockTextureIndex overlayTextureIndex = texture->overlay.getBlockTextureIndex(m_textureMethodParams[face][O_INDEX], blockColor[O_INDEX]);
    BlockTextureIndex overlayNormTextureIndex = texture->overlay.getNormalTextureIndex(m_textureMethodParams[face][O_INDEX], blockColor[O_INDEX]);
    BlockTextureIndex overlayDispTextureIndex = texture->overlay.getDispTextureIndex(m_textureMethodParams[face][O_INDEX], blockColor[O_INDEX]);

    // TODO(Ben): Bitwise ops?
    ui8 baseTextureAtlas = (ui8)(baseTextureIndex / ATLAS_SIZE);
    ui8 baseNormTextureAtlas = (ui8)(baseNormTextureIndex / ATLAS_SIZE);
    ui8 baseDispTextureAtlas = (ui8)(baseDispTextureIndex / ATLAS_SIZE);
    ui8 overlayTextureAtlas = (ui8)(overlayTextureIndex / ATLAS_SIZE);
    ui8 overlayNormTextureAtlas = (ui8)(overlayNormTextureIndex / ATLAS_SIZE);
    ui8 overlayDispTextureAtlas = (ui8)(overlayDispTextureIndex / ATLAS_SIZE);
    baseTextureIndex %= ATLAS_SIZE;
    baseNormTextureIndex %= ATLAS_SIZE;
    baseDispTextureIndex %= ATLAS_SIZE;
    overlayTextureIndex %= ATLAS_SIZE;
    overlayNormTextureIndex %= ATLAS_SIZE;
    overlayDispTextureIndex %= ATLAS_SIZE;
    i32v3 pos(bx, by, bz);
    int uOffset = (ui8)(pos[FACE_AXIS[face][0]] * FACE_AXIS_SIGN[face][0]);
    int vOffset = (ui8)(pos[FACE_AXIS[face][1]] * FACE_AXIS_SIGN[face][1]);

    // Construct the quad
    i16 quadIndex = quads.size();
    quads.emplace_back();
    m_numQuads++;
    VoxelQuad* quad = &quads.back();
    quad->v0.mesherFlags = MESH_FLAG_ACTIVE;

    for (int i = 0; i < 4; i++) {
        BlockVertex& v = quad->verts[i];
        v.position = VoxelMesher::VOXEL_POSITIONS[face][i] + voxelPosOffset;
#ifdef USE_AO
        f32& ao = ambientOcclusion[i];
        v.color.r = (ui8)(blockColor[B_INDEX].r * ao);
        v.color.g = (ui8)(blockColor[B_INDEX].g * ao);
        v.color.b = (ui8)(blockColor[B_INDEX].b * ao);
        v.overlayColor.r = (ui8)(blockColor[O_INDEX].r * ao);
        v.overlayColor.g = (ui8)(blockColor[O_INDEX].g * ao);
        v.overlayColor.b = (ui8)(blockColor[O_INDEX].b * ao);
#else
        v.color = blockColor[B_INDEX];
        v.overlayColor = blockColor[O_INDEX];
#endif
        // TODO(Ben) array?
        v.textureIndex = baseTextureIndex;
        v.textureAtlas = baseTextureAtlas;
        v.normIndex = baseNormTextureIndex;
        v.normAtlas = baseNormTextureAtlas;
        v.dispIndex = baseDispTextureIndex;
        v.dispAtlas = baseDispTextureAtlas;
        v.overlayTextureIndex = overlayTextureIndex;
        v.overlayTextureAtlas = overlayTextureAtlas;
        v.overlayNormIndex = overlayNormTextureIndex;
        v.overlayNormAtlas = overlayNormTextureAtlas;
        v.overlayDispIndex = overlayDispTextureIndex;
        v.overlayDispAtlas = overlayDispTextureAtlas;
        
        v.textureWidth = (ui8)texture->base.size.x;
        v.textureHeight = (ui8)texture->base.size.y;
        v.overlayTextureWidth = (ui8)texture->overlay.size.x;
        v.overlayTextureHeight = (ui8)texture->overlay.size.y;
        v.blendMode = blendMode;
        v.face = (ui8)face;
    }
    // Set texture coordinates
    quad->verts[0].tex.x = (ui8)(UV_0 + uOffset);
    quad->verts[0].tex.y = (ui8)(UV_1 + vOffset);
    quad->verts[1].tex.x = (ui8)(UV_0 + uOffset);
    quad->verts[1].tex.y = (ui8)(UV_0 + vOffset);
    quad->verts[2].tex.x = (ui8)(UV_1 + uOffset);
    quad->verts[2].tex.y = (ui8)(UV_0 + vOffset);
    quad->verts[3].tex.x = (ui8)(UV_1 + uOffset);
    quad->verts[3].tex.y = (ui8)(UV_1 + vOffset);

    // Check against lowest and highest for culling in render
    // TODO(Ben): Think about this more
    if (quad->v0.position.x < m_lowestX) m_lowestX = quad->v0.position.x;
    if (quad->v0.position.x > m_highestX) m_highestX = quad->v0.position.x;
    if (quad->v0.position.y < m_lowestY) m_lowestY = quad->v0.position.y;
    if (quad->v0.position.y > m_highestY) m_highestY = quad->v0.position.y;
    if (quad->v0.position.z < m_lowestZ) m_lowestZ = quad->v0.position.z;
    if (quad->v0.position.z > m_highestZ) m_highestZ = quad->v0.position.z;

    { // Look-Behind Greedy Merging(tm)
        if (quad->v0 == quad->v3 && quad->v1 == quad->v2) {
            quad->v0.mesherFlags |= MESH_FLAG_MERGE_RIGHT;
            ui16 leftIndex = m_quadIndices[blockIndex + leftOffset][face];
            // Check left merge
            if (leftIndex != NO_QUAD_INDEX) {
                VoxelQuad& lQuad = quads[leftIndex];
                if (((lQuad.v0.mesherFlags & MESH_FLAG_MERGE_RIGHT) != 0) &&
                    lQuad.v0.position[frontAxis] == quad->v0.position[frontAxis] &&
                    lQuad.v1.position[frontAxis] == quad->v1.position[frontAxis] &&
                    lQuad.v0 == quad->v0 && lQuad.v1 == quad->v1) {
                    // Stretch the previous quad
                    lQuad.verts[rightStretchIndex].position[rightAxis] += QUAD_SIZE;
                    lQuad.verts[rightStretchIndex].tex.x += texOffset.x;
                    lQuad.verts[rightStretchIndex + 1].position[rightAxis] += QUAD_SIZE;
                    lQuad.verts[rightStretchIndex + 1].tex.x += texOffset.x;
                    // Remove the current quad
                    quads.pop_back();
                    m_numQuads--;
                    quadIndex = leftIndex;
                    quad = &lQuad;
                }
            }
        }
        // Check back merge
        if (quad->v0 == quad->v1 && quad->v2 == quad->v3) {
            quad->v0.mesherFlags |= MESH_FLAG_MERGE_FRONT;
            int backIndex = m_quadIndices[blockIndex + backOffset][face];
            if (backIndex != NO_QUAD_INDEX) {
                VoxelQuad* bQuad = &quads[backIndex];
                while (!(bQuad->v0.mesherFlags & MESH_FLAG_ACTIVE)) {
                    backIndex = bQuad->replaceQuad;
                    bQuad = &quads[backIndex];
                }
                if (((bQuad->v0.mesherFlags & MESH_FLAG_MERGE_FRONT) != 0) &&
                    bQuad->v0.position[rightAxis] == quad->v0.position[rightAxis] &&
                    bQuad->v2.position[rightAxis] == quad->v2.position[rightAxis] &&
                    bQuad->v0 == quad->v0 && bQuad->v1 == quad->v1) {
                    bQuad->v0.position[frontAxis] += QUAD_SIZE;
                    bQuad->v0.tex.y += texOffset.y;
                    bQuad->v3.position[frontAxis] += QUAD_SIZE;
                    bQuad->v3.tex.y += texOffset.y;
                    quadIndex = backIndex;
                    // Mark as not in use
                    quad->v0.mesherFlags = 0;
                    quad->replaceQuad = backIndex;
                    m_numQuads--;
                }
            }
        }
    }
    
    // Mark quadIndices so we can merge this quad later
    m_quadIndices[blockIndex][face] = quadIndex;
}

//adds a flora mesh
void ChunkMesher::addFlora() {

    //const Block &block = (*m_blocks)[mi.btype];
    //mi.currentBlock = &block;

    //ColorRGB8 color, overlayColor;
    //const int wc = mi.wc;
    //const int btype = mi.btype;

    ////Biome *biome = mi.chunkGridData->heightData[mi.nz*CHUNK_WIDTH + mi.nx].biome;
    //int temperature = mi.chunkGridData->heightData[mi.nz*CHUNK_WIDTH + mi.nx].temperature;
    //int rainfall = mi.chunkGridData->heightData[mi.nz*CHUNK_WIDTH + mi.nx].rainfall;

    //GLuint flags = GETFLAGS(m_blockData[mi.wc]);

    //ui8 sunLight = m_sunData[wc];
    //ColorRGB8 lampLight((m_lampData[wc] & LAMP_RED_MASK) >> LAMP_RED_SHIFT,
    //                    (m_lampData[wc] & LAMP_GREEN_MASK) >> LAMP_GREEN_SHIFT,
    //                    m_lampData[wc] & LAMP_BLUE_MASK);

    //sunLight = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - sunLight)));
    //lampLight.r = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.r)));
    //lampLight.g = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.g)));
    //lampLight.b = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.b)));

    //(*m_blocks)[btype].GetBlockColor(color, overlayColor, flags, temperature, rainfall, block.textures[0]);

    ////We will offset the texture index based on the texture method
    //i32 textureIndex = block.textures[0]->base.getBlockTextureIndex(mi.pxBaseMethodParams, color);
    //i32 overlayTextureIndex = block.textures[0]->overlay.getBlockTextureIndex(mi.pxOverlayMethodParams, overlayColor);

    //int r;

    //switch (mi.meshType){
    //    case MeshType::LEAVES:

    //        break;
    //    case MeshType::CROSSFLORA:
    //        //Generate a random number between 0 and 3 inclusive
    //        r = std::bind(std::uniform_int_distribution<int>(0, 1), std::mt19937(/*getPositionSeed(mi.nx, mi.nz)*/))();

    //        _cutoutVerts.resize(_cutoutVerts.size() + 8);
    //        VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::crossFloraVertices[r], VoxelMesher::floraNormals, 0, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
    //        mi.cutoutIndex += 4;
    //        VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::crossFloraVertices[r], VoxelMesher::floraNormals, 12, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
    //        mi.cutoutIndex += 4;
    //        break;
    //    case MeshType::FLORA:
    //        //Generate a random number between 0 and 3 inclusive
    //        r = std::bind(std::uniform_int_distribution<int>(0, 3), std::mt19937(/*getPositionSeed(mi.nx, mi.nz)*/))();

    //        _cutoutVerts.resize(_cutoutVerts.size() + 12);
    //        VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::floraVertices[r], VoxelMesher::floraNormals, 0, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
    //        mi.cutoutIndex += 4;
    //        VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::floraVertices[r], VoxelMesher::floraNormals, 12, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
    //        mi.cutoutIndex += 4;
    //        VoxelMesher::makeFloraFace(&(_cutoutVerts[0]), VoxelMesher::floraVertices[r], VoxelMesher::floraNormals, 24, block.waveEffect, i32v3(mi.nx, mi.ny, mi.nz), mi.cutoutIndex, textureIndex, overlayTextureIndex, color, overlayColor, sunLight, lampLight, block.textures[0]);
    //        mi.cutoutIndex += 4;
    //        break;
    //}
}

//Gets the liquid level from a block index
#define LEVEL(i) ((_blockIDData[i] == 0) ? 0 : (((nextBlock = &GETBLOCK(_blockIDData[i]))->caIndex == block.caIndex) ? nextBlock->waterMeshLevel : 0))

#define CALCULATE_LIQUID_VERTEX_HEIGHT(height, heightA, heightB, cornerIndex) \
    div = 0; \
    tot = 0; \
    if (heightA) { \
        tot += heightA; \
        div++; \
    } \
    \
    if (heightB) { \
        tot += heightB; \
        div++; \
    } \
    \
    if (div) { \
        int lvl = LEVEL(cornerIndex); \
        if (lvl) { \
            tot += lvl; \
            div++; \
        } \
        height = (height + (tot / (float)maxLevel)) / (div + 1); \
    } 
    
   
//END CALCULATE_LIQUID_VERTEX_HEIGHT

// TODO(Ben): Instead of 8 verts per box, share vertices and index into it!
void ChunkMesher::addLiquid() {
  //  const Block &block = (*m_blocks)[mi.btype];
  //  Block* nextBlock;
  //  i32 nextBlockID;
  //  const i32 wc = mi.wc;
  //  i32 x = mi.x;
  //  i32 y = mi.y;
  //  i32 z = mi.z;

  //  RenderTask* task = mi.task;

  //  const i32 maxLevel = 100;

  //  float liquidLevel = block.waterMeshLevel / (float)maxLevel;
  //  float fallingReduction = 0.0f;

  //  bool faces[6] = { false, false, false, false, false, false };

  //  float backLeftHeight = liquidLevel;
  //  float backRightHeight = liquidLevel;
  //  float frontRightHeight = liquidLevel;
  //  float frontLeftHeight = liquidLevel;

  //  const i32 MIN_ALPHA = 75;
  //  const i32 MAX_ALPHA = 175;
  //  const i32 ALPHA_RANGE = MAX_ALPHA - MIN_ALPHA;

  //  ui8 backRightAlpha, frontRightAlpha, frontLeftAlpha, backLeftAlpha;

  //  i32 textureUnit = 0;

  //  i32 div;
  //  i32 tot;

  //  i32 left, right, back, front, bottom;

  //  ui8 uOff = x * 7;
  //  ui8 vOff = 224 - z * 7;

  //  ui8 temperature = chunkGridData->heightData[x + z*CHUNK_WIDTH].temperature;
  //  ui8 depth = chunkGridData->heightData[x + z*CHUNK_WIDTH].depth;

  //  ColorRGB8 color;// = GameManager::texturePackLoader->getColorMap(TerrainGenerator::DefaultColorMaps::WATER)[depth * 256 + temperature];

  //  ui8 sunlight = _sunlightData[wc];
  //  ColorRGB8 lampLight((_lampLightData[wc] & LAMP_RED_MASK) >> LAMP_RED_SHIFT,
  //                      (_lampLightData[wc] & LAMP_GREEN_MASK) >> LAMP_GREEN_SHIFT,
  //                      _lampLightData[wc] & LAMP_BLUE_MASK);

  //  sunlight = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - sunlight)));
  //  lampLight.r = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.r)));
  //  lampLight.g = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.g)));
  //  lampLight.b = (ui8)(255.0f*(LIGHT_OFFSET + pow(LIGHT_MULT, MAXLIGHT - lampLight.b)));

  //  nextBlockID = _blockIDData[wc + PADDED_OFFSETS::BOTTOM];
  //  nextBlock = &GETBLOCK(nextBlockID);
  //  //Check if the block is falling
  //  if (nextBlockID == 0 || nextBlock->waterBreak || (nextBlock->caIndex == block.caIndex && nextBlock->waterMeshLevel != maxLevel)) {
  //      memset(faces, 1, 6); //all faces are active
  ////      backLeftHeight = backRightHeight = frontLeftHeight = frontRightHeight 
  //      fallingReduction = 1.0f;
  //  } else {

  //      //Get occlusion
  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::LEFT]);
  //      faces[XNEG] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));

  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::BACK]);
  //      faces[ZNEG] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));

  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::RIGHT]);
  //      faces[XPOS] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));

  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::FRONT]);
  //      faces[ZPOS] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));

  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::BOTTOM]);
  //      faces[YNEG] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));
  //  }

  //  left = LEVEL(wc + PADDED_OFFSETS::LEFT);
  //  right = LEVEL(wc + PADDED_OFFSETS::RIGHT);
  //  back = LEVEL(wc + PADDED_OFFSETS::BACK);
  //  front = LEVEL(wc + PADDED_OFFSETS::FRONT);
  //  bottom = LEVEL(wc + PADDED_OFFSETS::BOTTOM);

  //  //Calculate the liquid levels

  //  //Back Left Vertex
  //  CALCULATE_LIQUID_VERTEX_HEIGHT(backLeftHeight, left, back, wc + PADDED_OFFSETS::BACK_LEFT);

  //  //Back Right Vertex
  //  CALCULATE_LIQUID_VERTEX_HEIGHT(backRightHeight, right, back, wc + PADDED_OFFSETS::BACK_RIGHT);

  //  //Front Right Vertex
  //  CALCULATE_LIQUID_VERTEX_HEIGHT(frontRightHeight, right, front, wc + PADDED_OFFSETS::FRONT_RIGHT);

  //  //Front Left Vertex
  //  CALCULATE_LIQUID_VERTEX_HEIGHT(frontLeftHeight, left, front, wc + PADDED_OFFSETS::FRONT_LEFT);

  //  //only occlude top if we are a full water block and our sides arent down at all
  //  if (liquidLevel == 1.0f && backRightHeight == 1.0f && backLeftHeight == 1.0f && frontLeftHeight == 1.0f && frontRightHeight == 1.0f) {
  //      nextBlock = &GETBLOCK(_blockIDData[wc + PADDED_OFFSETS::TOP]);
  //      faces[YPOS] = ((nextBlock->caIndex != block.caIndex) && (nextBlock->occlude == BlockOcclusion::NONE));
  //  } else {
  //      faces[YPOS] = true;
  //  }
  //  
  //  //Compute alpha
  //  if (bottom == maxLevel) {
  //      backRightAlpha = backLeftAlpha = frontRightAlpha = frontLeftAlpha = MAX_ALPHA;
  //  } else {
  //      backRightAlpha = (ui8)(backRightHeight * ALPHA_RANGE + MIN_ALPHA);
  //      backLeftAlpha = (ui8)(backLeftHeight * ALPHA_RANGE + MIN_ALPHA);
  //      frontRightAlpha = (ui8)(frontRightHeight * ALPHA_RANGE + MIN_ALPHA);
  //      frontLeftAlpha = (ui8)(frontLeftHeight * ALPHA_RANGE + MIN_ALPHA);
  //  }

  //  //Add vertices for the faces
  //  if (faces[YNEG]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);
  //      
  //      _waterVboVerts[mi.liquidIndex].position[0] = x + VoxelMesher::liquidVertices[48];
  //      _waterVboVerts[mi.liquidIndex].position[1] = y + VoxelMesher::liquidVertices[49] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex].position[2] = z + VoxelMesher::liquidVertices[50];
  //      _waterVboVerts[mi.liquidIndex + 1].position[0] = x + VoxelMesher::liquidVertices[51];
  //      _waterVboVerts[mi.liquidIndex + 1].position[1] = y + VoxelMesher::liquidVertices[52] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 1].position[2] = z + VoxelMesher::liquidVertices[53];
  //      _waterVboVerts[mi.liquidIndex + 2].position[0] = x + VoxelMesher::liquidVertices[54];
  //      _waterVboVerts[mi.liquidIndex + 2].position[1] = y + VoxelMesher::liquidVertices[55] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 2].position[2] = z + VoxelMesher::liquidVertices[56];
  //      _waterVboVerts[mi.liquidIndex + 3].position[0] = x + VoxelMesher::liquidVertices[57];
  //      _waterVboVerts[mi.liquidIndex + 3].position[1] = y + VoxelMesher::liquidVertices[58] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 3].position[2] = z + VoxelMesher::liquidVertices[59];

  //      //set alpha
  //      _waterVboVerts[mi.liquidIndex].color.a = backLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = backRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = frontRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = backLeftAlpha;

  //      mi.liquidIndex += 4;
  //  }

  //  if (faces[ZPOS]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);

  //      _waterVboVerts[mi.liquidIndex].position[0] = x + VoxelMesher::liquidVertices[0];
  //      _waterVboVerts[mi.liquidIndex].position[1] = y + frontLeftHeight;
  //      _waterVboVerts[mi.liquidIndex].position[2] = z + VoxelMesher::liquidVertices[2];
  //      _waterVboVerts[mi.liquidIndex + 1].position[0] = x + VoxelMesher::liquidVertices[3];
  //      _waterVboVerts[mi.liquidIndex + 1].position[1] = y + VoxelMesher::liquidVertices[4] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 1].position[2] = z + VoxelMesher::liquidVertices[5];
  //      _waterVboVerts[mi.liquidIndex + 2].position[0] = x + VoxelMesher::liquidVertices[6];
  //      _waterVboVerts[mi.liquidIndex + 2].position[1] = y + VoxelMesher::liquidVertices[7] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 2].position[2] = z + VoxelMesher::liquidVertices[8];
  //      _waterVboVerts[mi.liquidIndex + 3].position[0] = x + VoxelMesher::liquidVertices[9];
  //      _waterVboVerts[mi.liquidIndex + 3].position[1] = y + frontRightHeight;
  //      _waterVboVerts[mi.liquidIndex + 3].position[2] = z + VoxelMesher::liquidVertices[11];

  //      _waterVboVerts[mi.liquidIndex].color.a = frontLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = frontLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = frontRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = frontRightAlpha;

  //      mi.liquidIndex += 4;
  //  }

  //  if (faces[YPOS]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);

  //      _waterVboVerts.resize(_waterVboVerts.size() + 4);
  //      _waterVboVerts[mi.liquidIndex].position[0] = x + VoxelMesher::liquidVertices[24];
  //      _waterVboVerts[mi.liquidIndex].position[1] = y + backLeftHeight;
  //      _waterVboVerts[mi.liquidIndex].position[2] = z + VoxelMesher::liquidVertices[26];
  //      _waterVboVerts[mi.liquidIndex + 1].position[0] = x + VoxelMesher::liquidVertices[27];
  //      _waterVboVerts[mi.liquidIndex + 1].position[1] = y + frontLeftHeight;
  //      _waterVboVerts[mi.liquidIndex + 1].position[2] = z + VoxelMesher::liquidVertices[29];
  //      _waterVboVerts[mi.liquidIndex + 2].position[0] = x + VoxelMesher::liquidVertices[30];
  //      _waterVboVerts[mi.liquidIndex + 2].position[1] = y + frontRightHeight;
  //      _waterVboVerts[mi.liquidIndex + 2].position[2] = z + VoxelMesher::liquidVertices[32];
  //      _waterVboVerts[mi.liquidIndex + 3].position[0] = x + VoxelMesher::liquidVertices[33];
  //      _waterVboVerts[mi.liquidIndex + 3].position[1] = y + backRightHeight;
  //      _waterVboVerts[mi.liquidIndex + 3].position[2] = z + VoxelMesher::liquidVertices[35];

  //      _waterVboVerts[mi.liquidIndex].color.a = backLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = frontLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = frontRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = backRightAlpha;

  //      mi.liquidIndex += 4;
  //  }

  //  if (faces[ZNEG]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);

  //      _waterVboVerts[mi.liquidIndex].position[0] = x + VoxelMesher::liquidVertices[60];
  //      _waterVboVerts[mi.liquidIndex].position[1] = y + backRightHeight;
  //      _waterVboVerts[mi.liquidIndex].position[2] = z + VoxelMesher::liquidVertices[62];
  //      _waterVboVerts[mi.liquidIndex + 1].position[0] = x + VoxelMesher::liquidVertices[63];
  //      _waterVboVerts[mi.liquidIndex + 1].position[1] = y + VoxelMesher::liquidVertices[64] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 1].position[2] = z + VoxelMesher::liquidVertices[65];
  //      _waterVboVerts[mi.liquidIndex + 2].position[0] = x + VoxelMesher::liquidVertices[66];
  //      _waterVboVerts[mi.liquidIndex + 2].position[1] = y + VoxelMesher::liquidVertices[67] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 2].position[2] = z + VoxelMesher::liquidVertices[68];
  //      _waterVboVerts[mi.liquidIndex + 3].position[0] = x + VoxelMesher::liquidVertices[69];
  //      _waterVboVerts[mi.liquidIndex + 3].position[1] = y + backLeftHeight;
  //      _waterVboVerts[mi.liquidIndex + 3].position[2] = z + VoxelMesher::liquidVertices[71];

  //      _waterVboVerts[mi.liquidIndex].color.a = backRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = backRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = backLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = backLeftAlpha;

  //      mi.liquidIndex += 4;
  //  }
  //  if (faces[XPOS]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);

  //      _waterVboVerts[mi.liquidIndex].position.x = x + VoxelMesher::liquidVertices[12];
  //      _waterVboVerts[mi.liquidIndex].position.y = y + frontRightHeight;
  //      _waterVboVerts[mi.liquidIndex].position.z = z + VoxelMesher::liquidVertices[14];
  //      _waterVboVerts[mi.liquidIndex + 1].position.x = x + VoxelMesher::liquidVertices[15];
  //      _waterVboVerts[mi.liquidIndex + 1].position.y = y + VoxelMesher::liquidVertices[16] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 1].position.z = z + VoxelMesher::liquidVertices[17];
  //      _waterVboVerts[mi.liquidIndex + 2].position.x = x + VoxelMesher::liquidVertices[18];
  //      _waterVboVerts[mi.liquidIndex + 2].position.y = y + VoxelMesher::liquidVertices[19] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 2].position.z = z + VoxelMesher::liquidVertices[20];
  //      _waterVboVerts[mi.liquidIndex + 3].position.x = x + VoxelMesher::liquidVertices[21];
  //      _waterVboVerts[mi.liquidIndex + 3].position.y = y + backRightHeight;
  //      _waterVboVerts[mi.liquidIndex + 3].position.z = z + VoxelMesher::liquidVertices[23];

  //      _waterVboVerts[mi.liquidIndex].color.a = frontRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = frontRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = backRightAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = backRightAlpha;

  //      mi.liquidIndex += 4;
  //  }
  //  if (faces[XNEG]){

  //      VoxelMesher::makeLiquidFace(_waterVboVerts, mi.liquidIndex, uOff, vOff, lampLight, sunlight, color, textureUnit);

  //      _waterVboVerts[mi.liquidIndex].position[0] = x + VoxelMesher::liquidVertices[36];
  //      _waterVboVerts[mi.liquidIndex].position[1] = y + backLeftHeight;
  //      _waterVboVerts[mi.liquidIndex].position[2] = z + VoxelMesher::liquidVertices[38];
  //      _waterVboVerts[mi.liquidIndex + 1].position[0] = x + VoxelMesher::liquidVertices[39];
  //      _waterVboVerts[mi.liquidIndex + 1].position[1] = y + VoxelMesher::liquidVertices[40] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 1].position[2] = z + VoxelMesher::liquidVertices[41];
  //      _waterVboVerts[mi.liquidIndex + 2].position[0] = x + VoxelMesher::liquidVertices[42];
  //      _waterVboVerts[mi.liquidIndex + 2].position[1] = y + VoxelMesher::liquidVertices[43] - fallingReduction;
  //      _waterVboVerts[mi.liquidIndex + 2].position[2] = z + VoxelMesher::liquidVertices[44];
  //      _waterVboVerts[mi.liquidIndex + 3].position[0] = x + VoxelMesher::liquidVertices[45];
  //      _waterVboVerts[mi.liquidIndex + 3].position[1] = y + frontLeftHeight;
  //      _waterVboVerts[mi.liquidIndex + 3].position[2] = z + VoxelMesher::liquidVertices[47];

  //      _waterVboVerts[mi.liquidIndex].color.a = backLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 1].color.a = backLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 2].color.a = frontLeftAlpha;
  //      _waterVboVerts[mi.liquidIndex + 3].color.a = frontLeftAlpha;

  //      mi.liquidIndex += 4;
  //  }
}

int ChunkMesher::getLiquidLevel(int blockIndex, const Block& block) {
    int val = GETBLOCKID(blockData[blockIndex]); // Get block ID
    val = val - block.liquidStartID;
    if (val < 0) return 0;
    if (val > block.liquidLevels) return 0;
    return val;
}

bool ChunkMesher::shouldRenderFace(int offset) {
    const Block& neighbor = blocks->operator[](blockData[blockIndex + offset]);
    if (neighbor.occlude == BlockOcclusion::ALL) return false;
    if ((neighbor.occlude == BlockOcclusion::SELF) && (blockID == neighbor.ID)) return false;
    return true;
}

int ChunkMesher::getOcclusion(const Block& block) {
    if (block.occlude == BlockOcclusion::ALL) return 1;
    if ((block.occlude == BlockOcclusion::SELF) && (blockID == block.ID)) return 1;
    return 0;
}

ui8 ChunkMesher::getBlendMode(const BlendType& blendType) {
    // Shader interprets this with bitwise ops
    ubyte blendMode = 0x14; //0x14 = 00 01 01 00
    switch (blendType) {
        case BlendType::ALPHA:
            blendMode |= 1; //Sets blendMode to 00 01 01 01
            break;
        case BlendType::ADD:
            blendMode += 4; //Sets blendMode to 00 01 10 00
            break;
        case BlendType::SUBTRACT:
            blendMode -= 4; //Sets blendMode to 00 01 00 00
            break;
        case BlendType::MULTIPLY:
            blendMode -= 16; //Sets blendMode to 00 00 01 00
            break;
    }
    return blendMode;
}


void ChunkMesher::buildTransparentVao(ChunkMesh& cm) {
    glGenVertexArrays(1, &(cm.transVaoID));
    glBindVertexArray(cm.transVaoID);

    glBindBuffer(GL_ARRAY_BUFFER, cm.transVboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cm.transIndexID);

    for (int i = 0; i < 8; i++) {
        glEnableVertexAttribArray(i);
    }

    //position + texture type
    glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), 0);
    //UV, animation, blendmode
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (4)));
    //textureAtlas_textureIndex
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (8)));
    //Texture dimensions
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (12)));
    //color
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (16)));
    //overlayColor
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (20)));
    //lightcolor[3], sunlight,
    glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (24)));
    //normal
    glVertexAttribPointer(7, 3, GL_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (28)));

    glBindVertexArray(0);
}

void ChunkMesher::buildCutoutVao(ChunkMesh& cm) {
    glGenVertexArrays(1, &(cm.cutoutVaoID));
    glBindVertexArray(cm.cutoutVaoID);

    glBindBuffer(GL_ARRAY_BUFFER, cm.cutoutVboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ChunkRenderer::sharedIBO);

    for (int i = 0; i < 8; i++) {
        glEnableVertexAttribArray(i);
    }

    //position + texture type
    glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), 0);
    //UV, animation, blendmode
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (4)));
    //textureAtlas_textureIndex
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (8)));
    //Texture dimensions
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (12)));
    //color
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (16)));
    //overlayColor
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (20)));
    //lightcolor[3], sunlight,
    glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (24)));
    //normal
    glVertexAttribPointer(7, 3, GL_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (28)));


    glBindVertexArray(0);
}

void ChunkMesher::buildVao(ChunkMesh& cm) {
    glGenVertexArrays(1, &(cm.vaoID));
    glBindVertexArray(cm.vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, cm.vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ChunkRenderer::sharedIBO);

    for (int i = 0; i < 8; i++) {
        glEnableVertexAttribArray(i);
    }

    // vPosition_Face
    glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), offsetptr(BlockVertex, position));
    // vTex_Animation_BlendMode
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), offsetptr(BlockVertex, tex));
    // .0
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), offsetptr(BlockVertex, textureAtlas));
    // vNDTextureAtlas
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), offsetptr(BlockVertex, normAtlas));
    // vNDTextureIndex
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), offsetptr(BlockVertex, normIndex));
    // vTexDims
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), offsetptr(BlockVertex, textureWidth));
    // vColor
    glVertexAttribPointer(6, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), offsetptr(BlockVertex, color));
    // vOverlayColor
    glVertexAttribPointer(7, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), offsetptr(BlockVertex, overlayColor));

    glBindVertexArray(0);
}

void ChunkMesher::buildWaterVao(ChunkMesh& cm) {
    glGenVertexArrays(1, &(cm.waterVaoID));
    glBindVertexArray(cm.waterVaoID);
    glBindBuffer(GL_ARRAY_BUFFER, cm.waterVboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ChunkRenderer::sharedIBO);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, cm.waterVboID);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LiquidVertex), 0);
    //uvs_texUnit_texIndex
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (12)));
    //color
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (16)));
    //light
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (20)));

    glBindVertexArray(0);
}