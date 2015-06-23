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
    void addBlock();
    void addQuad(int face, int rightAxis, int frontAxis, int leftOffset, int backOffset, int rightStretchIndex, f32 ambientOcclusion[]);
    void computeAmbientOcclusion(int upOffset, int frontOffset, int rightOffset, f32 ambientOcclusion[]);
    void addFlora();
    void addLiquid(MesherInfo& mi);

    int getLiquidLevel(int blockIndex, const Block& block);

    bool shouldRenderFace(int offset);
    int getOcclusion(const Block& block);

    std::vector<BlockVertex> m_finalVerts[6];

    std::vector<VoxelQuad> m_quads[6];
    ui32 m_numQuads;

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
    ui8v3 m_voxelPosOffset;

    int m_highestY;
    int m_lowestY;
    int m_highestX;
    int m_lowestX;
    int m_highestZ;
    int m_lowestZ;

    Chunk* chunk; ///< The chunk we are currently meshing;
    std::shared_ptr<ChunkGridData> chunkGridData; ///< current grid data

    int wSize;
    // Voxel data arrays
    ui16 m_quadIndices[PADDED_CHUNK_SIZE][6];
    ui16 m_wvec[CHUNK_SIZE];
    ui16 m_blockData[PADDED_CHUNK_SIZE];
    ui16 m_tertiaryData[PADDED_CHUNK_SIZE];

    ui32 m_finalQuads[7000];

    BlockVertex m_topVerts[4100];

    const BlockPack* m_blocks = nullptr;
};