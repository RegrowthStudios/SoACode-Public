#pragma once
#include <algorithm>
#include <queue>
#include <set>
#include <mutex>

#include "ChunkRenderer.h"
#include "FloraGenerator.h"
#include "SmartVoxelContainer.h"
#include "readerwriterqueue.h"
#include "WorldStructs.h"
#include "VoxelLightEngine.h"

//used by threadpool

const int MAXLIGHT = 31;

class Block;
struct PlantData;

enum LightTypes {LIGHT, SUNLIGHT};

enum class ChunkStates { LOAD, GENERATE, SAVE, LIGHT, TREES, MESH, WATERMESH, DRAW, INACTIVE }; //more priority is lower

struct LightMessage;
struct RenderTask;

//For lamp colors. Used to extract color values from the 16 bit color code
#define LAMP_RED_MASK 0x7C00
#define LAMP_GREEN_MASK 0x3E0
#define LAMP_BLUE_MASK 0x1f
#define LAMP_RED_SHIFT 10
#define LAMP_GREEN_SHIFT 5
//no blue shift

class Chunk{
public:

    friend class ChunkManager;
    friend class EditorTree;
    friend class ChunkMesher;
    friend class ChunkIOManager;
    friend class CAEngine;
    friend class ChunkGenerator;
    friend class ChunkUpdater;
    friend class VoxelLightEngine;
    friend class PhysicsEngine;
    friend class RegionFileManager;

    void init(const glm::ivec3 &pos, int hzI, int hxI, FaceData *fd);
    
    void changeState(ChunkStates State);
    
    int getLeftBlockData(int c);
    int getLeftBlockData(int c, int x, int *c2, Chunk **owner);
    int getRightBlockData(int c);
    int getRightBlockData(int c, int x, int *c2, Chunk **owner);
    int getFrontBlockData(int c);
    int getFrontBlockData(int c, int z, int *c2, Chunk **owner);
    int getBackBlockData(int c);
    int getBackBlockData(int c, int z, int *c2, Chunk **owner);
    int getBottomBlockData(int c);
    int getBottomBlockData(int c, int y, int *c2, Chunk **owner);
    int getTopBlockData(int c);
    int getTopBlockData(int c, int *c2, Chunk **owner);
    int getTopBlockData(int c, int y, int *c2, Chunk **owner);

    int getTopSunlight(int c);

    void getLeftLightData(int c, GLbyte &l, GLbyte &sl);
    void getRightLightData(int c, GLbyte &l, GLbyte &sl);
    void getFrontLightData(int c, GLbyte &l, GLbyte &sl);
    void getBackLightData(int c, GLbyte &l, GLbyte &sl);
    void getBottomLightData(int c, GLbyte &l, GLbyte &sl);
    void getTopLightData(int c, GLbyte &l, GLbyte &sl);

    void clear(bool clearDraw = 1);
    void clearBuffers();
    
    void CheckEdgeBlocks();
    int GetPlantType(int x, int z, Biome *biome);

    void SetupMeshData(RenderTask *renderTask);

    Chunk(){
    }
    ~Chunk(){
        clearBuffers();
    }

    static vector <MineralData*> possibleMinerals;
    
    //getters
    ChunkStates getState() const { return _state; }
    GLushort getBlockData(int c) const;
    int getBlockID(int c) const;
    int getSunlight(int c) const;

    ui16 getLampLight(int c) const;
    ui16 getLampRed(int c) const;
    ui16 getLampGreen(int c) const;
    ui16 getLampBlue(int c) const;

    const Block& getBlock(int c) const;
    int getRainfall(int xz) const;
    int getTemperature(int xz) const;

    static ui16 getLampRedFromHex(ui16 color) { return (color & LAMP_RED_MASK) >> LAMP_RED_SHIFT; }
    static ui16 getLampGreenFromHex(ui16 color) { return (color & LAMP_GREEN_MASK) >> LAMP_GREEN_SHIFT; }
    static ui16 getLampBlueFromHex(ui16 color) { return color & LAMP_BLUE_MASK; }

    int getLevelOfDetail() const { return _levelOfDetail; }

    //setters
    void setBlockID(int c, int val);
    void setBlockData(int c, ui16 val);
    void setSunlight(int c, ui8 val);
    void setLampLight(int c, ui16 val);

    void setLevelOfDetail(int lod) { _levelOfDetail = lod; }
    

    int neighbors;
    bool activeUpdateList[8];
    bool drawWater;
    bool hasLoadedSunlight;
    bool occlude; //this needs a new name
    bool topBlocked, leftBlocked, rightBlocked, bottomBlocked, frontBlocked, backBlocked;
    bool dirty;
    int loadStatus;
    bool inLoadThread;
    volatile bool inSaveThread;
    volatile bool inRenderThread;
    bool inGenerateThread;
    bool inFinishedMeshes;
    bool inFinishedChunks;
    bool isAccessible;

    ChunkMesh *mesh;

    std::vector <TreeData> treesToLoad;
    std::vector <PlantData> plantsToLoad;
    std::vector <GLushort> spawnerBlocks;
    glm::ivec3 position;
    FaceData faceData;
    int hzIndex, hxIndex;
    int worldX, worlxY, worldZ;
    int numBlocks;
    int minh;
    double distance2;
    bool freeWaiting;

    int blockUpdateIndex;
    int treeTryTicks;
    int drawIndex, updateIndex;
    int threadJob;
    float setupWaitingTime;

    std::vector <ui16> blockUpdateList[8][2];

    //Even though these are vectors, they are treated as fifo usually, and when not, it doesn't matter
    std::vector <SunlightUpdateNode> sunlightUpdateQueue;
    std::vector <SunlightRemovalNode> sunlightRemovalQueue;
    std::vector <LampLightUpdateNode> lampLightUpdateQueue;
    std::vector <LampLightRemovalNode> lampLightRemovalQueue;

    std::vector <ui16> sunRemovalList;
    std::vector <ui16> sunExtendList;

    static ui32 vboIndicesID;

    std::vector <Chunk *> *setupListPtr;
    Chunk *right, *left, *front, *back, *top, *bottom;

    //Main thread locks this when modifying chunks, meaning some readers, such as the chunkIO thread, should lock this before reading.
    static std::mutex modifyLock;



private:
    ChunkStates _state;

    //The data that defines the voxels
    SmartVoxelContainer<ui16> _blockIDContainer;
    SmartVoxelContainer<ui8> _sunlightContainer;
    SmartVoxelContainer<ui16> _lampLightContainer;

    ui8 _biomes[CHUNK_LAYER]; //lookup for biomesLookupMap
    ui8 _temperatures[CHUNK_LAYER];
    ui8 _rainfalls[CHUNK_LAYER];
    ui8 _depthMap[CHUNK_LAYER];

    int _levelOfDetail;

};

//INLINE FUNCTION DEFINITIONS
#include "Chunk.inl"

struct ChunkSlot
{
    void Initialize(const glm::ivec3 &pos, Chunk *ch, int Ipos, int Jpos, FaceData *fD){
        chunk = ch;
        position = pos;
        ipos = Ipos;
        jpos = Jpos;
        fd = fD;
    }

    Chunk *chunk;
    glm::ivec3 position;

    //squared distance
    double distance2;
    int ipos, jpos;
    FaceData *fd;
};