#pragma once
#include <algorithm>
#include <queue>
#include <set>

#include "ChunkRenderer.h"
#include "FloraGenerator.h"
#include "VoxelIntervalTree.h"
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

    Chunk() : _data(nullptr), _sunlightData(nullptr), _lampLightData(nullptr){
    }
    ~Chunk(){
        clearBuffers();
    }

    static vector <MineralData*> possibleMinerals;
    
    //getters
    ChunkStates getState() const { return state; }
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

    //setters
    void setBlockID(int c, int val);
    void setBlockData(int c, ui16 val);
    void setSunlight(int c, int val);
    void setLampLight(int c, ui16 val);

    static ui16 getLampRedFromHex(int color) { return (color & 0x7C00) >> 10; }
    static ui16 getLampGreenFromHex(int color) { return (color & 0x3E0) >> 5; }
    static ui16 getLampBlueFromHex(int color) { return color & 0x1F; }

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

    //Even though these are vectors, they are treated as fifo usually, and when not, it doesn't matter
    vector <SunlightUpdateNode> sunlightUpdateQueue;
    vector <SunlightRemovalNode> sunlightRemovalQueue;
    vector <LampLightUpdateNode> lampLightUpdateQueue;
    vector <LampLightRemovalNode> lampLightRemovalQueue;

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

    //The data that defines the voxels
    VoxelIntervalTree<ui16> _dataTree;
    VoxelIntervalTree<ui8> _sunlightTree;
    VoxelIntervalTree<ui16> _lampLightTree;
    ui16 *_data; 
    ui8 *_sunlightData;
    //Voxel light data is only allocated when needed
    ui8 *_lampLightData;

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