#pragma once
#include "OpenGLStructs.h"
#include "BlockData.h"
#include "ChunkMesh.h"

class RenderTask;
class Chunk;
class ChunkMeshData;
class BlockPack;
struct BlockTexture;
class BlockTextureLayer;

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
    enum FACES { XNEG, XPOS, YNEG, YPOS, ZNEG, ZPOS };

    void mergeTopVerts(MesherInfo& mi);
    void mergeFrontVerts(MesherInfo& mi);
    void mergeBackVerts(MesherInfo& mi);
    void mergeRightVerts(MesherInfo& mi);
    void mergeLeftVerts(MesherInfo& mi);
    void mergeBottomVerts(MesherInfo& mi);

    void addBlockToMesh(MesherInfo& mi);
    void addBlockXFace()

    void addFloraToMesh(MesherInfo& mi);
    void addLiquidToMesh(MesherInfo& mi);

    int getLiquidLevel(int blockIndex, const Block& block);

    bool checkBlockFaces(bool faces[6], const RenderTask* task, const BlockOcclusion occlude, const i32 btype, const i32 wc);
    GLubyte calculateSmoothLighting(int accumulatedLight, int numAdjacentBlocks);
    void calculateLampColor(ColorRGB8& dst, ui16 src0, ui16 src1, ui16 src2, ui16 src3, ui8 numAdj);
    void calculateFaceLight(BlockVertex* face, int blockIndex, int upOffset, int frontOffset, int rightOffset, f32 ambientOcclusion[]);

    void computeLODData(int levelOfDetail);

    std::vector<BlockVertex> _finalTopVerts;
    std::vector<BlockVertex> _finalLeftVerts;
    std::vector<BlockVertex> _finalRightVerts;
    std::vector<BlockVertex> _finalFrontVerts;
    std::vector<BlockVertex> _finalBackVerts;
    std::vector<BlockVertex> _finalBottomVerts;

    std::vector<BlockVertex> _vboVerts;
    std::vector<BlockVertex> _transparentVerts;
    std::vector<BlockVertex> _cutoutVerts;
    std::vector<LiquidVertex> _waterVboVerts;

    //Dimensions of the voxel data, based on LOD
    int dataWidth;
    int dataLayer;
    int dataSize;

    Chunk* chunk; ///< The chunk we are currently meshing;
    std::shared_ptr<ChunkGridData> chunkGridData; ///< current grid data

    int wSize;
    // Voxel data arrays
    ui16 m_wvec[CHUNK_SIZE];
    ui16 m_blockData[PADDED_CHUNK_SIZE];
    ui16 m_lampData[PADDED_CHUNK_SIZE];
    ui8 m_sunData[PADDED_CHUNK_SIZE];
    ui16 m_tertiaryData[PADDED_CHUNK_SIZE];

    ui32 m_finalQuads[7000];

    BlockVertex m_topVerts[4100];

    BlockVertex m_leftVerts[4100];
    i32 _currPrevLeftQuads;
    i32 _prevLeftQuads[2][1024];

    BlockVertex m_rightVerts[4100];
    i32 _currPrevRightQuads;
    i32 _prevRightQuads[2][1024];

    BlockVertex _frontVerts[4100];
    i32 _currPrevFrontQuads;
    i32 _prevFrontQuads[2][1024];

    BlockVertex _backVerts[4100];
    i32 _currPrevBackQuads;
    i32 _prevBackQuads[2][1024];

    BlockVertex _bottomVerts[4100];

    const BlockPack* m_blocks = nullptr;
};