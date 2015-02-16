#pragma once
#include "OpenGLStructs.h"
#include "BlockTextureMethods.h"

enum class MeshType {
    NONE, 
    BLOCK, 
    LEAVES, 
    FLORA,
    CROSSFLORA, 
    LIQUID,
    FLAT 
};

#define UNINITIALIZED_INDEX -1

enum class RenderTaskType;

class Block;
class Chunk;
class ChunkGridData;
class ChunkMesh;
class RenderTask;

// Stores Chunk Mesh Information
class MesherInfo {
public:

    void init(int dataWidth, int dataLayer);

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
    std::shared_ptr<ChunkGridData> chunkGridData;
    i32v3 position;
};

class ChunkMeshRenderData {
public:
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
    class ChunkMesh *chunkMesh = nullptr;
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

    ChunkMeshRenderData meshInfo;

    GLuint vboID = 0;
    GLuint vaoID = 0;
    GLuint transVboID = 0;
    GLuint transVaoID = 0;
    GLuint cutoutVboID = 0;
    GLuint cutoutVaoID = 0;
    GLuint waterVboID = 0;
    GLuint waterVaoID = 0;
    float distance2 = 32.0f;
    glm::ivec3 position;
    bool inFrustum = false;
    bool needsSort = true;
    bool needsDestroy = false;
    volatile int refCount = 0;
    int vecIndex = UNINITIALIZED_INDEX;

    //*** Transparency info for sorting ***
    GLuint transIndexID = 0;
    std::vector <i8v3> transQuadPositions;
    std::vector <ui32> transQuadIndices;
};
