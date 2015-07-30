#pragma once
#include "Vertex.h"
#include "BlockData.h"
#include "Chunk.h"
#include "ChunkMesh.h"
#include "ChunkMeshTask.h"

class BlockPack;
class BlockTextureLayer;
class ChunkMeshData;
struct BlockTexture;
struct PlanetHeightData;
struct FloraQuadData;

// Sizes For A Padded Chunk
const int PADDED_CHUNK_WIDTH = (CHUNK_WIDTH + 2);
const int PADDED_CHUNK_LAYER = (PADDED_CHUNK_WIDTH * PADDED_CHUNK_WIDTH);
const int PADDED_CHUNK_SIZE = (PADDED_CHUNK_LAYER * PADDED_CHUNK_WIDTH);

// each worker thread gets one of these
// This class is too big to statically allocate
class ChunkMesher {
public:
    void init(const BlockPack* blocks);

    // Easily creates chunk mesh synchronously.
    CALLER_DELETE ChunkMesh* easyCreateChunkMesh(const Chunk* chunk, MeshTaskType type) {
        prepareData(chunk);
        ChunkMesh* mesh = new ChunkMesh;
        mesh->position = chunk->getVoxelPosition().pos;
        uploadMeshData(*mesh, createChunkMeshData(type));
        return mesh;
    }

    // Call one of these before createChunkMesh
    void prepareData(const Chunk* chunk);
    // For use with threadpool
    void prepareDataAsync(Chunk* chunk);

    // TODO(Ben): Unique ptr?
    // Must call prepareData or prepareDataAsync first
    CALLER_DELETE ChunkMeshData* createChunkMeshData(MeshTaskType type);

    // Returns true if the mesh is renderable
    static bool uploadMeshData(ChunkMesh& mesh, ChunkMeshData* meshData);

    // Frees buffers AND deletes memory. mesh Pointer is invalid after calling.
    static void freeChunkMesh(CALLEE_DELETE ChunkMesh* mesh);

    void freeBuffers();

    int bx, by, bz; // Block iterators
    int blockIndex;
    ui16 blockID;
    const Block* block;
    const PlanetHeightData* heightData;
    ui8v3 voxelPosOffset;

    // Heightmap data
    PlanetHeightData heightDataBuffer[CHUNK_LAYER];
    // Voxel data arrays
    ui16 blockData[PADDED_CHUNK_SIZE];
    ui16 tertiaryData[PADDED_CHUNK_SIZE];

    const BlockPack* blocks = nullptr;

    VoxelPosition3D chunkVoxelPos;
private:
    void addBlock();
    void addQuad(int face, int rightAxis, int frontAxis, int leftOffset, int backOffset, int rightStretchIndex, const ui8v2& texOffset, f32 ambientOcclusion[]);
    void computeAmbientOcclusion(int upOffset, int frontOffset, int rightOffset, f32 ambientOcclusion[]);
    void addFlora();
    void addFloraQuad(const ui8v3* positions, FloraQuadData& data);
    int tryMergeQuad(VoxelQuad* quad, std::vector<VoxelQuad>& quads, int face, int rightAxis, int frontAxis, int leftOffset, int backOffset, int rightStretchIndex, const ui8v2& texOffset);
    void addLiquid();

    int getLiquidLevel(int blockIndex, const Block& block);

    bool shouldRenderFace(int offset);
    int getOcclusion(const Block& block);

    ui8 getBlendMode(const BlendType& blendType);

    static void buildTransparentVao(ChunkMesh& cm);
    static void buildCutoutVao(ChunkMesh& cm);
    static void buildVao(ChunkMesh& cm);
    static void buildWaterVao(ChunkMesh& cm);

    ui16 m_quadIndices[PADDED_CHUNK_SIZE][6];
    ui16 m_wvec[CHUNK_SIZE];

    std::vector<BlockVertex> m_finalVerts[6];

    std::vector<VoxelQuad> m_floraQuads;
    std::vector<VoxelQuad> m_quads[6];
    ui32 m_numQuads;

    BlockTextureMethodParams m_textureMethodParams[6][2];

    // TODO(Ben): Change this up a bit
    std::vector<LiquidVertex> _waterVboVerts;

    ChunkMeshData* m_chunkMeshData = nullptr;

    int m_highestY;
    int m_lowestY;
    int m_highestX;
    int m_lowestX;
    int m_highestZ;
    int m_lowestZ;

    const PlanetHeightData* m_chunkHeightData;

    static PlanetHeightData defaultChunkHeightData[CHUNK_LAYER];

    int wSize;

    ui32 m_finalQuads[7000];

    BlockVertex m_topVerts[4100];

};