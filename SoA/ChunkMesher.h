#pragma once
#include "Vertex.h"
#include "BlockData.h"
#include "ChunkMesh.h"

class BlockPack;
class BlockTextureLayer;
class Chunk;
class ChunkMeshData;
class RenderTask;
struct BlockTexture;
struct PlanetHeightData;

// Sizes For A Padded Chunk
const int PADDED_CHUNK_WIDTH = (CHUNK_WIDTH + 2);
const int PADDED_CHUNK_LAYER = (PADDED_CHUNK_WIDTH * PADDED_CHUNK_WIDTH);
const int PADDED_CHUNK_SIZE = (PADDED_CHUNK_LAYER * PADDED_CHUNK_WIDTH);

struct VoxelQuad {
    union {
        struct {
            BlockVertex v0;
            BlockVertex v1;
            BlockVertex v2;
            BlockVertex v3;
        };
        struct {
            BlockVertex verts[4];
        };
    };
};

// each worker thread gets one of these
class ChunkMesher {
    friend class RenderTask;
public:
    void init(const BlockPack* blocks);

    bool createChunkMesh(RenderTask* renderTask);
    bool createOnlyWaterMesh(RenderTask* renderTask);
    void freeBuffers();

    static void bindVBOIndicesID();

    ChunkMeshData* chunkMeshData = nullptr;
private:
    // Cardinal?
    enum FACES { XNEG, XPOS, YNEG, YPOS, ZNEG, ZPOS };

    void addBlock();
    void addQuad(int face, int leftOffset, int downOffset);
    void addFlora();
    void addLiquid(MesherInfo& mi);

    int getLiquidLevel(int blockIndex, const Block& block);

    bool shouldRenderFace(int offset);

    std::vector<BlockVertex> m_finalVerts[6];

    std::vector<BlockVertex> _vboVerts;
    std::vector<BlockVertex> _transparentVerts;
    std::vector<BlockVertex> _cutoutVerts;
    std::vector<LiquidVertex> _waterVboVerts;

    //Dimensions of the voxel data, based on LOD
    int m_dataWidth;
    int m_dataLayer;
    int m_dataSize;

    int bx, by, bz; // Block iterators
    int m_blockIndex;
    ui16 m_blockID;
    const Block* m_block;
    const PlanetHeightData* m_heightData;

    Chunk* chunk; ///< The chunk we are currently meshing;
    std::shared_ptr<ChunkGridData> chunkGridData; ///< current grid data

    int wSize;
    // Voxel data arrays
    i32 m_quadIndices[PADDED_CHUNK_SIZE][6];
    ui16 m_wvec[CHUNK_SIZE];
    ui16 m_blockData[PADDED_CHUNK_SIZE];
    ui16 m_tertiaryData[PADDED_CHUNK_SIZE];

    ui32 m_finalQuads[7000];

    BlockVertex m_topVerts[4100];

    const BlockPack* m_blocks = nullptr;
};