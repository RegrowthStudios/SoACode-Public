#pragma once
#include <algorithm>
#include <queue>
#include <set>
#include <mutex>

#include <boost/circular_buffer_fwd.hpp>

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

    void init(const i32v3 &gridPos, int hzI, int hxI, FaceData *fd, ChunkSlot* Owner);
    
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
    i32v3 gridPosition;  // Position relative to the voxel grid
    i32v3 chunkPosition; // floor(gridPosition / (float)CHUNK_WIDTH)
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

    boost::circular_buffer<Chunk*> *setupListPtr;
    Chunk *right, *left, *front, *back, *top, *bottom;

    //Main thread locks this when modifying chunks, meaning some readers, such as the chunkIO thread, should lock this before reading.
    static std::mutex modifyLock;

    ChunkSlot* owner;
    ChunkGridData* chunkGridData;

private:
    ChunkStates _state;

    //The data that defines the voxels
    SmartVoxelContainer<ui16> _blockIDContainer;
    SmartVoxelContainer<ui8> _sunlightContainer;
    SmartVoxelContainer<ui16> _lampLightContainer;

    int _levelOfDetail;

};

//INLINE FUNCTION DEFINITIONS
#include "Chunk.inl"

class ChunkSlot
{
public:

    friend class ChunkManager;

    ChunkSlot(const glm::ivec3 &pos, Chunk *ch, int Ipos, int Jpos, FaceData *fD) :
        chunk(ch),
        position(pos),
        ipos(Ipos),
        jpos(Jpos),
        faceData(fD),
        left(-1),
        right(-1),
        back(-1),
        front(-1),
        bottom(-1),
        top(-1),
        vecIndex(-2),
        numNeighbors(0){}

    inline void calculateDistance2(const i32v3& cameraPos) {
        distance2 = getDistance2(position, cameraPos);
        chunk->distance2 = distance2;
    }

    void clearNeighbors(vector <ChunkSlot>& chunkSlots) {
        ChunkSlot* cs;
        if (left != -1) {
            cs = &chunkSlots[left];
            if (cs->right == vecIndex) {
                cs->right = -1;
                cs->numNeighbors--;
            }
        }
        if (right != -1) {
            cs = &chunkSlots[right];
            if (cs->left == vecIndex) {
                cs->left = -1;
                cs->numNeighbors--;
            }
        }
        if (top != -1) {
            cs = &chunkSlots[top];
            if (cs->bottom == vecIndex) {
                cs->bottom = -1;
                cs->numNeighbors--;
            }
        }
        if (bottom != -1) {
            cs = &chunkSlots[bottom];
            if (cs->top == vecIndex) {
                cs->top = -1;
                cs->numNeighbors--;
            }
        }
        if (front != -1) {
            cs = &chunkSlots[front];
            if (cs->back == vecIndex) {
                cs->back = -1;
                cs->numNeighbors--;
            }
        }
        if (back != -1) {
            cs = &chunkSlots[back];
            if (cs->front == vecIndex) {
                cs->front = -1;
                cs->numNeighbors--;
            }
        }
        numNeighbors = 0;
        left = right = top = bottom = back = front = -1;
    }

    void detectNeighbors(vector <ChunkSlot>& chunkSlots, std::unordered_map<i32v3, int>& chunkSlotIndexMap) {
       
        std::unordered_map<i32v3, int>::iterator it;

        i32v3 chPos;
        chPos.x = fastFloor(position.x / (float)CHUNK_WIDTH);
        chPos.y = fastFloor(position.y / (float)CHUNK_WIDTH);
        chPos.z = fastFloor(position.z / (float)CHUNK_WIDTH);
        ChunkSlot* cs;

        //left
        if (left == -1) {
            it = chunkSlotIndexMap.find(chPos + i32v3(-1, 0, 0));
            if (it != chunkSlotIndexMap.end()) {
                left = it->second;
                cs = &chunkSlots[left];
                cs->right = vecIndex;
                numNeighbors++;
                cs->numNeighbors++;
            }
        }
        //right
        if (right == -1) {
            it = chunkSlotIndexMap.find(chPos + i32v3(1, 0, 0));
            if (it != chunkSlotIndexMap.end()) {
                right = it->second;
                cs = &chunkSlots[right];
                cs->left = vecIndex;
                numNeighbors++;
                cs->numNeighbors++;
            }
        }

        //back
        if (back == -1) {
            it = chunkSlotIndexMap.find(chPos + i32v3(0, 0, -1));
            if (it != chunkSlotIndexMap.end()) {
                back = it->second;
                cs = &chunkSlots[back];
                cs->front = vecIndex;
                numNeighbors++;
                cs->numNeighbors++;
            }
        }

        //front
        if (front == -1) {
            it = chunkSlotIndexMap.find(chPos + i32v3(0, 0, 1));
            if (it != chunkSlotIndexMap.end()) {
                front = it->second;
                cs = &chunkSlots[front];
                cs->back = vecIndex;
                numNeighbors++;
                cs->numNeighbors++;
            }
        }

        //bottom
        if (bottom == -1) {
            it = chunkSlotIndexMap.find(chPos + i32v3(0, -1, 0));
            if (it != chunkSlotIndexMap.end()) {
                bottom = it->second;
                cs = &chunkSlots[bottom];
                cs->top = vecIndex;
                numNeighbors++;
                cs->numNeighbors++;
            }
        }

        //top
        if (top == -1) {
            it = chunkSlotIndexMap.find(chPos + i32v3(0, 1, 0));
            if (it != chunkSlotIndexMap.end()) {
                top = it->second;
                cs = &chunkSlots[top];
                cs->bottom = vecIndex;
                numNeighbors++;
                cs->numNeighbors++;
            }
        }
    }

    Chunk *chunk;
    glm::ivec3 position;

    int numNeighbors;
    int vecIndex;
    //Indices of neighbors
    int left, right, back, front, top, bottom;

    //squared distance
    double distance2;
    int ipos, jpos;
    FaceData *faceData;
private:
    static double getDistance2(const i32v3& pos, const i32v3& cameraPos) {
        double dx = (cameraPos.x <= pos.x) ? pos.x : ((cameraPos.x > pos.x + CHUNK_WIDTH) ? (pos.x + CHUNK_WIDTH) : cameraPos.x);
        double dy = (cameraPos.y <= pos.y) ? pos.y : ((cameraPos.y > pos.y + CHUNK_WIDTH) ? (pos.y + CHUNK_WIDTH) : cameraPos.y);
        double dz = (cameraPos.z <= pos.z) ? pos.z : ((cameraPos.z > pos.z + CHUNK_WIDTH) ? (pos.z + CHUNK_WIDTH) : cameraPos.z);
        dx = dx - cameraPos.x;
        dy = dy - cameraPos.y;
        dz = dz - cameraPos.z;
        //we dont sqrt the distance since sqrt is slow
        return dx*dx + dy*dy + dz*dz;
    }
};