#pragma once
#include <algorithm>
#include <queue>
#include <set>
#include <mutex>

#include <boost/circular_buffer.hpp>

#include "Vorb.h"
#include "IVoxelMapper.h"

#include "ChunkRenderer.h"
#include "FloraGenerator.h"
#include "SmartVoxelContainer.h"
#include "readerwriterqueue.h"
#include "WorldStructs.h"
#include "Vorb.h"
#include "VoxelBits.h"
#include "VoxelLightEngine.h"

//used by threadpool

const int MAXLIGHT = 31;

class Block;
struct PlantData;
namespace vorb {
    namespace core {
        class IThreadPoolTask;
    }
}

enum LightTypes {LIGHT, SUNLIGHT};

enum class ChunkStates { LOAD, GENERATE, SAVE, LIGHT, TREES, MESH, WATERMESH, DRAW, INACTIVE }; //more priority is lower

struct LightMessage;
class RenderTask;
class ChunkMesher;

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

    void updateContainers() {
        _blockIDContainer.update(_dataLock);
        _sunlightContainer.update(_dataLock);
        _lampLightContainer.update(_dataLock);
        _tertiaryDataContainer.update(_dataLock);
    }
    
    void changeState(ChunkStates State);
    
    /// Checks if adjacent chunks are in thread, since we dont want
    /// to remove chunks when their neighbors need them.
    bool isAdjacentInThread();

    // Helper functions for getting neighbor data. They will lock if
    // accessing neighbor chunks, and will return -1 if neighbor is null.
    int getLeftBlockData(int blockIndex);
    int getLeftBlockData(int blockIndex, int x, int& nextBlockIndex, Chunk*& owner);
    int getRightBlockData(int blockIndex);
    int getRightBlockData(int blockIndex, int x, int& nextBlockIndex, Chunk*& owner);
    int getFrontBlockData(int blockIndex);
    int getFrontBlockData(int blockIndex, int z, int& nextBlockIndex, Chunk*& owner);
    int getBackBlockData(int blockIndex);
    int getBackBlockData(int blockIndex, int z, int& nextBlockIndex, Chunk*& owner);
    int getBottomBlockData(int blockIndex);
    int getBottomBlockData(int blockIndex, int y, int& nextBlockIndex, Chunk*& owner);
    int getTopBlockData(int blockIndex);
    int getTopBlockData(int blockIndex, int& nextBlockIndex, Chunk*& owner);
    int getTopBlockData(int blockIndex, int y, int& nextBlockIndex, Chunk*& owner);

    int getTopSunlight(int c);

    void clear(bool clearDraw = 1);
    void clearBuffers();
    void clearNeighbors();
    
    void CheckEdgeBlocks();
    int GetPlantType(int x, int z, Biome *biome);

    void setupMeshData(ChunkMesher *chunkMesher);

    void addToChunkList(boost::circular_buffer<Chunk*> *chunkListPtr);
    void clearChunkListPtr();

    bool hasCaUpdates(int index);

    /// Constructor
    /// @param shortRecycler: Recycler for ui16 data arrays
    /// @param byteRecycler: Recycler for ui8 data arrays
    Chunk(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler, 
          vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8>* byteRecycler) : 
          _blockIDContainer(shortRecycler), 
          _sunlightContainer(byteRecycler),
          _lampLightContainer(shortRecycler),
          _tertiaryDataContainer(shortRecycler) {
        // Empty
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
    ui16 getTertiaryData(int c) const;
    int getFloraHeight(int c) const;

    ui16 getLampLight(int c) const;
    ui16 getLampRed(int c) const;
    ui16 getLampGreen(int c) const;
    ui16 getLampBlue(int c) const;

    const Block& getBlock(int c) const;
    int getRainfall(int xz) const;
    int getTemperature(int xz) const;

    int getLevelOfDetail() const { return _levelOfDetail; }

    //setters
    void setBlockID(int c, int val);
    void setBlockData(int c, ui16 val);
    void setTertiaryData(int c, ui16 val);
    void setSunlight(int c, ui8 val);
    void setLampLight(int c, ui16 val);
    void setFloraHeight(int c, ui16 val);

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
    volatile bool isAccessible;

    vcore::IThreadPoolTask* lastOwnerTask; ///< Pointer to task that is working on us

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

    std::queue <SunlightUpdateNode> sunlightUpdateQueue;
    std::queue <SunlightRemovalNode> sunlightRemovalQueue;
    std::queue <LampLightUpdateNode> lampLightUpdateQueue;
    std::queue <LampLightRemovalNode> lampLightRemovalQueue;

    std::vector <ui16> sunRemovalList;
    std::vector <ui16> sunExtendList;

    static ui32 vboIndicesID;

    Chunk *right, *left, *front, *back, *top, *bottom;

    ChunkSlot* owner;
    ChunkGridData* chunkGridData;
    vvoxel::VoxelMapData* voxelMapData;

    // Thread safety functions
    inline void lock() { _dataLock.lock(); }
    inline void unlock() { _dataLock.unlock(); }
    std::mutex& getDataLock() { return _dataLock; }

private:

    /// _dataLock guards chunk data.
    /// Since only the main thread modifies data, the main thread does not
    /// need to lock when reading, only when writing. All other threads should
    /// lock when reading.
    std::mutex _dataLock; ///< Lock that guards chunk data. Try to minimize locking.

    // Keeps track of which setup list we belong to
    boost::circular_buffer<Chunk*> *_chunkListPtr;

    ChunkStates _state;

    //The data that defines the voxels
    vvoxel::SmartVoxelContainer<ui16> _blockIDContainer;
    vvoxel::SmartVoxelContainer<ui8> _sunlightContainer;
    vvoxel::SmartVoxelContainer<ui16> _lampLightContainer;
    vvoxel::SmartVoxelContainer<ui16> _tertiaryDataContainer;

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