#pragma once
#include "Vertex.h"
#include "BlockTextureMethods.h"
#include "ChunkHandle.h"
#include <Vorb/io/Keg.h>
#include <Vorb/graphics/gtypes.h>

enum class MeshType {
    NONE, 
    BLOCK, 
    LEAVES, 
    TRIANGLE,
    CROSSFLORA, 
    LIQUID,
    FLAT 
};
KEG_ENUM_DECL(MeshType);

enum class MeshTaskType;

class Block;
class Chunk;
class ChunkGridData;
class ChunkMesh;
class ChunkMeshTask;

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

struct VoxelQuad {
    VoxelQuad() {}

    VoxelQuad(VoxelQuad const &that)
    {
        for(size_t i=0; i<4; i++)
            verts[i]=that.verts[i];
    }

    ~VoxelQuad()
    {
        v.v0.BlockVertex::~BlockVertex();
        v.v1.BlockVertex::~BlockVertex();
        v.v2.BlockVertex::~BlockVertex();
        v.v3.BlockVertex::~BlockVertex(); 
    }

    VoxelQuad& operator=(const VoxelQuad& rhs) {
        for (size_t i = 0; i < 4; ++i) {
            verts[i] = rhs.verts[i];
        }
        return *this;
    }

    union {
        struct{
            BlockVertex v0;
            BlockVertex v1;
            BlockVertex v2;
            union {
                BlockVertex v3;
                ui16 replaceQuad; // Quad that replaces this quad
            };
        } v;
        BlockVertex verts[4];
    };
};

class ChunkMeshData
{
public:
    ChunkMeshData();
    ChunkMeshData(MeshTaskType type);

    void addTransQuad(const i8v3& pos);

    ChunkMeshRenderData chunkMeshRenderData;

    // TODO(Ben): Could use a contiguous buffer for this?
    std::vector <VoxelQuad> opaqueQuads;
    std::vector <VoxelQuad> transQuads;
    std::vector <VoxelQuad> cutoutQuads;
    std::vector <LiquidVertex> waterVertices;
    MeshTaskType type;

    //*** Transparency info for sorting ***
    ui32 transVertIndex = 0;
    std::vector <i8v3> transQuadPositions;
    std::vector <ui32> transQuadIndices;
};

#define ACTIVE_MESH_INDEX_NONE UINT_MAX

class ChunkMesh
{
public:
    ChunkMesh() : vboID(0), waterVboID(0),
        cutoutVboID(0), transVboID(0),
        vaoID(0), transVaoID(0),
        cutoutVaoID(0), waterVaoID(0) {}

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
    ui32 activeMeshesIndex = ACTIVE_MESH_INDEX_NONE; ///< Index into active meshes array
    ui32 updateVersion;
    bool inFrustum = false;
    bool needsSort = true;
    ChunkID id;

    //*** Transparency info for sorting ***
    VGIndexBuffer transIndexID = 0;
    std::vector<i8v3> transQuadPositions;
    std::vector<ui32> transQuadIndices;
};
