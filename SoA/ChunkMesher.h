#pragma once
#include "OpenGLStructs.h"
#include "BlockData.h"
#include "ChunkMesh.h"

class RenderTask;
class Chunk;
struct ChunkMeshData;
struct BlockTexture;
struct BlockTextureLayer;


// each worker thread gets one of these
class ChunkMesher {
public:
    ChunkMesher();
    ~ChunkMesher();
    
    bool createChunkMesh(RenderTask* renderTask);
    bool createOnlyWaterMesh(RenderTask* renderTask);
    void freeBuffers();

    ChunkMeshData* chunkMeshData;
private:
    enum FACES { XNEG, XPOS, YNEG, YPOS, ZNEG, ZPOS };

    inline void mergeTopVerts(MesherInfo& mi);
    inline void mergeFrontVerts(MesherInfo& mi);
    inline void mergeBackVerts(MesherInfo& mi);
    inline void mergeRightVerts(MesherInfo& mi);
    inline void mergeLeftVerts(MesherInfo& mi);
    inline void mergeBottomVerts(MesherInfo& mi);

    
    inline void getTextureIndex(const MesherInfo &mi, const BlockTextureLayer& blockTexture, int& result, int rightDir, int upDir, int frontDir, unsigned int directionIndex, ui8 color[3]);
    //inline void getOverlayTextureIndex(const MeshInfo &mi, const BlockTexture& blockTexture, int& result, int rightDir, int upDir, int frontDir, unsigned int directionIndex, ui8 overlayColor[3]);
    inline void getRandomTextureIndex(const MesherInfo &mi, const BlockTextureLayer& blockTexInfo, int& result);
    inline void getConnectedTextureIndex(const MesherInfo &mi, int& result, bool innerSeams, int rightDir, int upDir, int frontDir, unsigned int offset);
    inline void getGrassTextureIndex(const MesherInfo &mi, int& result, int rightDir, int upDir, int frontDir, unsigned int offset, ui8 color[3]);
    inline void getVerticalTextureIndex(const MesherInfo &mi, int& result, ConnectedTextureReducedMethod rm, int upDir, unsigned int offset);
    inline void getHorizontalTextureIndex(const MesherInfo &mi, int& result, bool innerSeams, int rightDir, int frontDir, unsigned int offset);


    void addBlockToMesh(MesherInfo& mi);
    void addFloraToMesh(MesherInfo& mi);
    void addLiquidToMesh(MesherInfo& mi);

    void bindVBOIndicesID();

    inline void GetLightDataArray(int c, int &x, int &y, int &z, ui8 lampLights[26][3], GLbyte sunlights[26], GLushort* chData, ui8* chSunData, ui16 *chLampData, bool faces[6]);
    inline bool checkBlockFaces(bool faces[6], ui8 lampLights[26][3], sbyte sunlights[26], const RenderTask* task, const bool occlude, const i32 btype, const i32 wc);
    inline GLubyte calculateSmoothLighting(int accumulatedLight, int numAdjacentBlocks);
    void calculateLampColor(ui8 dst[3], ui8 src0[3], ui8 src1[3], ui8 src2[3], ui8 src3[3], ui8 numAdj);

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
    ui32 _finalQuads[7000];

    BlockVertex _topVerts[4100];

    BlockVertex _leftVerts[4100];
    i32 _currPrevLeftQuads;
    i32 _prevLeftQuads[2][1024];

    BlockVertex _rightVerts[4100];
    i32 _currPrevRightQuads;
    i32 _prevRightQuads[2][1024];

    BlockVertex _frontVerts[4100];
    i32 _currPrevFrontQuads;
    i32 _prevFrontQuads[2][1024];

    BlockVertex _backVerts[4100];
    i32 _currPrevBackQuads;
    i32 _prevBackQuads[2][1024];

    BlockVertex _bottomVerts[4100];
};