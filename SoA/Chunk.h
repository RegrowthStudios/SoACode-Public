#pragma once
#include <algorithm>
#include <queue>
#include <set>
#include <mutex>

#include "Vorb.h"
#include <IThreadPoolTask.h>
#include "IVoxelMapper.h"

#include "Biome.h"
#include "ChunkRenderer.h"
#include "FloraGenerator.h"
#include "SmartVoxelContainer.h"
#include "readerwriterqueue.h"
#include "WorldStructs.h"
#include "TerrainGenerator.h"
#include "Vorb.h"
#include "VoxelBits.h"
#include "VoxelLightEngine.h"
#include "VoxPool.h"

//used by threadpool

const int MAXLIGHT = 31;

class Block;

class PlantData;

enum LightTypes {LIGHT, SUNLIGHT};

enum class ChunkStates { LOAD, GENERATE, SAVE, LIGHT, TREES, MESH, WATERMESH, DRAW, INACTIVE }; //more priority is lower

class LightMessage;
class RenderTask;
class CaPhysicsType;
class ChunkMesher;

class ChunkGridData {
public:
    ChunkGridData(vvox::VoxelMapData* VoxelMapData) : voxelMapData(VoxelMapData), refCount(1) {
        //Mark the data as unloaded
        heightData[0].height = UNLOADED_HEIGHT;
    }
    ~ChunkGridData() {
        delete voxelMapData;
    }
    vvox::VoxelMapData* voxelMapData;
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
    
    /// Checks if adjacent chunks are in thread, since we don't want
    /// to remove chunks when their neighbors need them.
    bool isAdjacentInThread();

    int getTopSunlight(int c);

    void clear(bool clearDraw = 1);
    void clearBuffers();
    void clearNeighbors();
    
    void CheckEdgeBlocks();
    int GetPlantType(int x, int z, Biome *biome);

    void setupMeshData(ChunkMesher *chunkMesher);

    void addToChunkList(std::vector<Chunk*> *chunkListPtr);
    void clearChunkListPtr();

    bool hasCaUpdates(const std::vector <CaPhysicsType*>& typesToUpdate);

    /// Conclassor
    /// @param shortRecycler: Recycler for ui16 data arrays
    /// @param byteRecycler: Recycler for ui8 data arrays
    Chunk(vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler, 
          vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8>* byteRecycler,
          int numCaTypes) : 
          _blockIDContainer(shortRecycler), 
          _sunlightContainer(byteRecycler),
          _lampLightContainer(shortRecycler),
          _tertiaryDataContainer(shortRecycler) {
        blockUpdateList.resize(numCaTypes * 2);
        activeUpdateList.resize(numCaTypes);
        // Empty
    }
    ~Chunk(){
        clearBuffers();
    }

    static std::vector<MineralData*> possibleMinerals;
    
    //getters
    ChunkStates getState() const { return _state; }
    ui16 getBlockData(int c) const;
    ui16 getBlockDataSafe(Chunk*& lockedChunk, int c);
    int getBlockID(int c) const;
    int getBlockIDSafe(Chunk*& lockedChunk, int c);
    int getSunlight(int c) const;
    int getSunlightSafe(int c, Chunk*& lockedChunk);
    ui16 getTertiaryData(int c) const;
    int getFloraHeight(int c) const;

    ui16 getLampLight(int c) const;
    ui16 getLampRed(int c) const;
    ui16 getLampGreen(int c) const;
    ui16 getLampBlue(int c) const;

    const Block& getBlock(int c) const;
    const Block& getBlockSafe(Chunk*& lockedChunk, int c);
    int getRainfall(int xz) const;
    int getTemperature(int xz) const;

    int getLevelOfDetail() const { return _levelOfDetail; }

    //setters
    void setBlockData(int c, ui16 val);
    void setBlockDataSafe(Chunk*& lockedChunk, int c, ui16 val);
    void setTertiaryData(int c, ui16 val);
    void setTertiaryDataSafe(Chunk*& lockedChunk, int c, ui16 val);
    void setSunlight(int c, ui8 val);
    void setSunlightSafe(Chunk*& lockedChunk, int c, ui8 val);
    void setLampLight(int c, ui16 val);
    void setLampLightSafe(Chunk*& lockedChunk, int c, ui16 val);
    void setFloraHeight(int c, ui16 val);
    void setFloraHeightSafe(Chunk*& lockedChunk, int c, ui16 val);

    void setLevelOfDetail(int lod) { _levelOfDetail = lod; }

    inline void addPhysicsUpdate(int caIndex, int blockIndex) {
        blockUpdateList[(caIndex << 1) + (int)activeUpdateList[caIndex]].push_back(blockIndex);
    }
    
    int numNeighbors;
    std::vector<bool> activeUpdateList;
    bool drawWater;
    bool hasLoadedSunlight;
    bool occlude; //this needs a new name
    bool topBlocked, leftBlocked, rightBlocked, bottomBlocked, frontBlocked, backBlocked;
    bool dirty;
    int loadStatus;
    volatile bool inLoadThread;
    volatile bool inSaveThread;
    volatile bool isAccessible;
    volatile bool queuedForMesh;

    bool queuedForPhysics;

    vorb::core::IThreadPoolTask<WorkerData>* lastOwnerTask; ///< Pointer to task that is working on us

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

    std::vector< std::vector<ui16> > blockUpdateList;

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
    vvox::VoxelMapData* voxelMapData;

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
    std::vector <Chunk*>* _chunkListPtr;

    ChunkStates _state;

    //The data that defines the voxels
    vvox::SmartVoxelContainer<ui16> _blockIDContainer;
    vvox::SmartVoxelContainer<ui8> _sunlightContainer;
    vvox::SmartVoxelContainer<ui16> _lampLightContainer;
    vvox::SmartVoxelContainer<ui16> _tertiaryDataContainer;

    int _levelOfDetail; ///< Determines the LOD of the chunk, 0 being base

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

inline i32 getPositionSeed(i32 x, i32 y, i32 z) {
    return ((x & 0x7FF) << 10) |
        ((y & 0x3FF)) |
        ((z & 0x7FF) << 21);
}
inline i32 getPositionSeed(i32 x, i32 z) {
    return ((x & 0xFFFF) << 16) |
        (z & 0xFFFF);
}
