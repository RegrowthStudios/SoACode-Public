#pragma once
#include "stdafx.h"
#include "OpenGLStructs.h"
#include "BlockTextureMethods.h"

#include <vector>

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

class RenderTask;
class Chunk;
class ChunkGridData;
class Block;

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
    ChunkGridData* chunkGridData;
};

struct ChunkMeshRenderData {
    ChunkMeshRenderData() : indexSize(0), waterIndexSize(0) {}
    i32 pxVboOff, pxVboSize, nxVboOff, nxVboSize, pzVboOff, pzVboSize, nzVboOff, nzVboSize;
    i32 pyVboOff, pyVboSize, nyVboOff, nyVboSize, transVboSize, cutoutVboSize;
    i32 highestY, lowestY, highestX, lowestX, highestZ, lowestZ;
    ui32 indexSize;
    ui32 waterIndexSize;
};

class ChunkMeshData
{
public:
    ChunkMeshData(Chunk *ch);
    ChunkMeshData(RenderTask *task);

    void addTransQuad(const i8v3& pos);

    ChunkMeshRenderData chunkMeshRenderData;

    std::vector <BlockVertex> vertices;
    std::vector <BlockVertex> transVertices;
    std::vector <BlockVertex> cutoutVertices;
    std::vector <LiquidVertex> waterVertices;
    Chunk *chunk;
    struct ChunkMesh *chunkMesh = nullptr;
    RenderTaskType type;

    //*** Transparency info for sorting ***
    ui32 transVertIndex;
    std::vector <i8v3> transQuadPositions;
    std::vector <ui32> transQuadIndices;
};

struct ChunkMesh
{
    ChunkMesh(Chunk *ch);

    ChunkMeshRenderData meshInfo;

    GLuint vboID = 0;
    GLuint vaoID = 0;
    GLuint transVboID = 0;
    GLuint transVaoID = 0;
    GLuint cutoutVboID = 0;
    GLuint cutoutVaoID = 0;
    GLuint waterVboID = 0;
    GLuint waterVaoID = 0;
    float distance;
    glm::ivec3 position;
    int vecIndex;
    bool inFrustum;
    bool needsSort;

    //*** Transparency info for sorting ***
    GLuint transIndexID;
    std::vector <i8v3> transQuadPositions;
    std::vector <ui32> transQuadIndices;

};
