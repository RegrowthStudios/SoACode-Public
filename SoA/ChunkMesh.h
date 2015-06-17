#pragma once
#include "OpenGLStructs.h"
#include "BlockTextureMethods.h"
#include <Vorb/graphics/gtypes.h>

enum class MeshType {
    NONE, 
    BLOCK, 
    LEAVES, 
    FLORA,
    CROSSFLORA, 
    LIQUID,
    FLAT 
};
KEG_ENUM_DECL(MeshType);

enum class RenderTaskType;

class Block;
class Chunk;
class NChunkGridData;
class ChunkMesh;
class RenderTask;

// Stores Chunk Mesh Information
class MesherInfo {
public:

    void init(BlockPack* blocks, int dataWidth, int dataLayer);

    // TODO(Ben): array this shit
    i32 index, topIndex, leftIndex, rightIndex, botIndex, backIndex, frontIndex, liquidIndex;
    i32 pLayerFrontIndex, pLayerBackIndex, pLayerLeftIndex, pLayerRightIndex;
    i32 wsize;
    i32 pyVboSize, nxVboSize, pxVboSize, nzVboSize, pzVboSize, nyVboSize, transparentIndex, cutoutIndex;
    i32 y, z, x;
    i32 y2, z2, x2; //used for transparent positions. == y*2,z*2,x*2
    i32 nx, ny, nz; //normal positions, as if it was at LOD 1.
    i32 wc;
    i32 btype;
    i32 pbtype;
    i32 pupIndex, pfrontIndex, pbackIndex, pbotIndex;
    i32 temperature, rainfall;
    i32 levelOfDetail;
    MeshType meshType;
    bool mergeUp, mergeBot, mergeFront, mergeBack;
    const Block* currentBlock;

    // Pointers to the voxel data
    ui16* blockIDData;
    ui16* lampLightData;
    ui8* sunlightData;
    ui16* tertiaryData;

    BlockTextureMethodParams pyBaseMethodParams, pyOverlayMethodParams;
    BlockTextureMethodParams nyBaseMethodParams, nyOverlayMethodParams;
    BlockTextureMethodParams pxBaseMethodParams, pxOverlayMethodParams;
    BlockTextureMethodParams nxBaseMethodParams, nxOverlayMethodParams;
    BlockTextureMethodParams pzBaseMethodParams, pzOverlayMethodParams;
    BlockTextureMethodParams nzBaseMethodParams, nzOverlayMethodParams;

    RenderTask* task;
    std::shared_ptr<NChunkGridData> chunkGridData;
    i32v3 position;
    BlockPack* blocks;
};

class ChunkMeshRenderData {
public:
    // TODO(Ben): These can be ui16
    i32 pxVboOff = 0;
    i32 pxVboSize = 0; 
    i32 nxVboOff = 0;
    i32 nxVboSize = 0;
    i32 pzVboOff = 0;
    i32 pzVboSize = 0;
    i32 nzVboOff = 0;
    i32 nzVboSize = 0;
    i32 pyVboOff = 0;
    i32 pyVboSize = 0;
    i32 nyVboOff = 0;
    i32 nyVboSize = 0;
    i32 transVboSize = 0;
    i32 cutoutVboSize = 0;
    i32 highestY = INT_MIN;
    i32 lowestY = INT_MAX;
    i32 highestX = INT_MIN;
    i32 lowestX = INT_MAX;
    i32 highestZ = INT_MIN;
    i32 lowestZ = INT_MAX;
    ui32 indexSize = 0;
    ui32 waterIndexSize = 0;
};

class ChunkMeshData
{
public:
    ChunkMeshData(ChunkMesh *cm);
    ChunkMeshData(RenderTask *task);

    void addTransQuad(const i8v3& pos);

    ChunkMeshRenderData chunkMeshRenderData;

    std::vector <BlockVertex> vertices;
    std::vector <BlockVertex> transVertices;
    std::vector <BlockVertex> cutoutVertices;
    std::vector <LiquidVertex> waterVertices;
    Chunk *chunk = nullptr;
    RenderTaskType type;

    //*** Transparency info for sorting ***
    ui32 transVertIndex = 0;
    std::vector <i8v3> transQuadPositions;
    std::vector <ui32> transQuadIndices;
};

class ChunkMesh
{
public:
    ChunkMesh(const Chunk *ch);
    ~ChunkMesh();

    ChunkMeshRenderData renderData;
    union {
        struct {
            VGVertexBuffer vboID;
            VGVertexBuffer waterVboID;
            VGVertexBuffer cutoutVboID;
            VGVertexBuffer transVboID;
        };
        VGVertexBuffer vbos[4];
    };
    union {
        struct {
            VGVertexArray vaoID;
            VGVertexArray transVaoID;
            VGVertexArray cutoutVaoID;
            VGVertexArray waterVaoID;
        };
        VGVertexArray vaos[4];
    };

    f64 distance2 = 32.0;
    f64v3 position;
    ui32 activeMeshesIndex; ///< Index into active meshes array
    bool inFrustum = false;
    bool needsSort = true;

    //*** Transparency info for sorting ***
    VGIndexBuffer transIndexID = 0;
    std::vector <i8v3> transQuadPositions;
    std::vector <ui32> transQuadIndices;
};
