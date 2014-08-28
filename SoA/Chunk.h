#pragma once
#include <algorithm>
#include <queue>
#include <set>

#include "ChunkRenderer.h"
#include "FloraGenerator.h"
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
struct LightRemovalNode;
struct RenderTask;

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
    ChunkStates getState() const { return state; }
    GLushort getBlockData(int c) const;
    int getBlockID(int c) const;
    int getLight(int type, int c) const;
    const Block& getBlock(int c) const;
    int getRainfall(int xz) const;
    int getTemperature(int xz) const;

    //setters
    void setBlockID(int c, int val);
    void setBlockData(int c, GLushort val);
    void setLight(int type, int c, int val);


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

    vector <TreeData> treesToLoad;
    vector <PlantData> plantsToLoad;
    vector <GLushort> spawnerBlocks;
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

    vector <ui16> blockUpdateList[8][2];
    vector <LightUpdateNode> lightUpdateQueue;
    vector <LightRemovalNode> lightRemovalQueue;
    vector <ui16> sunRemovalList;
    vector <ui16> sunExtendList;

    static ui32 vboIndicesID;

    vector <Chunk *> *setupListPtr;
    Chunk *right, *left, *front, *back, *top, *bottom;

private:
    ChunkStates state;

    //For passing light updates to neighbor chunks with multithreading.
    //One queue per edge so there is only one writer per queue
    //This is a queue of ints for compatability with moodycamel,
    //but the ints are interpreted as LightMessages
   /* moodycamel::ReaderWriterQueue<ui32> lightFromLeft;
    moodycamel::ReaderWriterQueue<ui32> lightFromRight;
    moodycamel::ReaderWriterQueue<ui32> lightFromFront;
    moodycamel::ReaderWriterQueue<ui32> lightFromBack;
    moodycamel::ReaderWriterQueue<ui32> lightFromTop;
    moodycamel::ReaderWriterQueue<ui32> lightFromBottom;
    //And one queue for the main thread updates, such as adding torches
    moodycamel::ReaderWriterQueue<ui32> lightFromMain; */


    ui16 data[CHUNK_SIZE]; 
    ui8 lightData[2][CHUNK_SIZE]; //0 = light 1 = sunlight
    ui8 biomes[CHUNK_LAYER]; //lookup for biomesLookupMap
    ui8 temperatures[CHUNK_LAYER];
    ui8 rainfalls[CHUNK_LAYER];
    ui8 depthMap[CHUNK_LAYER];

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