#pragma once
#include <algorithm>
#include <queue>
#include <set>
#include <mutex>

#include <boost/circular_buffer_fwd.hpp>

#include "Vorb.h"
#include "IVoxelMapper.h"

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

class ChunkGridData {
public:
    ChunkGridData(vvoxel::VoxelMapData* VoxelMapData) : voxelMapData(VoxelMapData), refCount(1) {
        //Mark the data as unloaded
        heightData[0].height = UNLOADED_HEIGHT;
    }
    ~ChunkGridData() {
        delete voxelMapData;
    }
    vvoxel::VoxelMapData* voxelMapData;
    HeightData heightData[CHUNK_LAYER];
    int refCount;
};

class ChunkSlot;

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

    void init(const i32v3 &gridPos, ChunkSlot* Owner);
    
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
    void clearNeighbors();
    
    void CheckEdgeBlocks();
    int GetPlantType(int x, int z, Biome *biome);

    void SetupMeshData(RenderTask *renderTask);

    void addToChunkList(boost::circular_buffer<Chunk*> *chunkListPtr);
    void removeFromChunkList();
    void clearChunkListPtr();

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
    
    int numNeighbors;
    bool activeUpdateList[8];
    bool drawWater;
    bool hasLoadedSunlight;
    bool occlude; //this needs a new name
    bool topBlocked, leftBlocked, rightBlocked, bottomBlocked, frontBlocked, backBlocked;
    bool dirty;
    int loadStatus;
    volatile bool inLoadThread;
    volatile bool inSaveThread;
    volatile bool inRenderThread;
    volatile bool inGenerateThread;
    volatile bool inFinishedMeshes;
    volatile bool inFinishedChunks;
    bool isAccessible;

    ChunkMesh *mesh;

    std::vector <TreeData> treesToLoad;
    std::vector <PlantData> plantsToLoad;
    std::vector <GLushort> spawnerBlocks;
    i32v3 gridPosition;  // Position relative to the voxel grid
    i32v3 chunkPosition; // floor(gridPosition / (float)CHUNK_WIDTH)

    int numBlocks;
    int minh;
    double distance2;
    bool freeWaiting;

    int blockUpdateIndex;
    int treeTryTicks;
    
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

    
    Chunk *right, *left, *front, *back, *top, *bottom;

    //Main thread locks this when modifying chunks, meaning some readers, such as the chunkIO thread, should lock this before reading.
    static std::mutex modifyLock;

    ChunkSlot* owner;
    ChunkGridData* chunkGridData;
    vvoxel::VoxelMapData* voxelMapData;

private:
    // Keeps track of which setup list we belong to, and where we are in the list.
    int _chunkListIndex;
    boost::circular_buffer<Chunk*> *_chunkListPtr;

    ChunkStates _state;

    //The data that defines the voxels
    SmartVoxelContainer<ui16> _blockIDContainer;
    SmartVoxelContainer<ui8> _sunlightContainer;
    SmartVoxelContainer<ui16> _lampLightContainer;
    SmartVoxelContainer<ui16> _tertiaryDataContainer;

    int _levelOfDetail;

};

//INLINE FUNCTION DEFINITIONS
#include "Chunk.inl"

class ChunkSlot
{
public:

    friend class ChunkManager;

    ChunkSlot(const glm::ivec3 &pos, Chunk *ch, ChunkGridData* cgd) :
        chunk(ch),
        position(pos),
        chunkGridData(cgd),
        left(nullptr),
        right(nullptr),
        back(nullptr),
        front(nullptr),
        bottom(nullptr),
        top(nullptr),
        numNeighbors(0),
        inFrustum(false),
        distance2(1.0f){}

    inline void calculateDistance2(const i32v3& cameraPos) {
        distance2 = getDistance2(position, cameraPos);
        chunk->distance2 = distance2;
    }

    void clearNeighbors();

    void detectNeighbors(std::unordered_map<i32v3, ChunkSlot*>& chunkSlotMap);

    void reconnectToNeighbors();

    Chunk *chunk;
    glm::ivec3 position;

    int numNeighbors;
    //Indices of neighbors
    ChunkSlot* left, *right, *back, *front, *top, *bottom;

    //squared distance
    double distance2;

    ChunkGridData* chunkGridData;

    bool inFrustum;
private:
    static double getDistance2(const i32v3& pos, const i32v3& cameraPos);
};