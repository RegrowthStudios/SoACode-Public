#pragma once
#include <algorithm>
#include <queue>
#include <set>
#include <mutex>

#include <Vorb/Vorb.h>
#include <Vorb/IThreadPoolTask.h>

#include "Biome.h"
#include "CAEngine.h"
#include "ChunkRenderer.h"
#include "FloraGenerator.h"
#include "SmartVoxelContainer.hpp"
#include "TerrainGenerator.h"
#include "VoxPool.h"
#include "VoxelBits.h"
#include "VoxelCoordinateSpaces.h"
#include "VoxelCoordinateSpaces.h"
#include "VoxelLightEngine.h"
#include "WorldStructs.h"
#include "readerwriterqueue.h"

#include <Vorb/RPC.h>

//used by threadpool

const int MAXLIGHT = 31;

class Block;

class PlantData;

enum LightTypes {LIGHT, SUNLIGHT};

enum class ChunkStates { LOAD, GENERATE, SAVE, LIGHT, TREES, MESH, WATERMESH, DRAW, INACTIVE }; //more priority is lower

class CaPhysicsType;
class ChunkMesher;
class LightMessage;
class RenderTask;
class SphericalTerrainGpuGenerator;

typedef int ChunkID;

class ChunkGridData {
public:
    ChunkGridData(const i32v2& pos, WorldCubeFace face) {
        gridPosition.pos = pos;
        gridPosition.face = face;
    }

    ChunkPosition2D gridPosition;
    int refCount = 1;
    HeightData heightData[CHUNK_LAYER];
    volatile bool wasRequestSent = false; /// True when heightmap was already sent for gen
    volatile bool isLoaded = false;
};

class Chunk{
public:
    friend class CAEngine;
    friend class ChunkGenerator;
    friend class ChunkIOManager;
    friend class ChunkListManager;
    friend class ChunkManager;
    friend class ChunkMemoryManager;
    friend class ChunkMesher;
    friend class ChunkUpdater;
    friend class EditorTree;
    friend class PhysicsEngine;
    friend class RegionFileManager;
    friend class SphericalVoxelComponentUpdater;
    friend class VoxelLightEngine;

    Chunk() {
        blockUpdateList.resize(CaPhysicsType::getNumCaTypes() * 2);
        activeUpdateList.resize(CaPhysicsType::getNumCaTypes());
    }

    void set(ChunkID cID,
             vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16>* shortRecycler,
             vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8>* byteRecycler) {
        id = cID;
        _blockIDContainer.setArrayRecycler(shortRecycler);
        _sunlightContainer.setArrayRecycler(byteRecycler);
        _lampLightContainer.setArrayRecycler(shortRecycler);
        _tertiaryDataContainer.setArrayRecycler(shortRecycler);
    }

    void init(const ChunkPosition3D &chunkPos);

    void updateContainers() {
        _blockIDContainer.update(_dataLock);
        _sunlightContainer.update(_dataLock);
        _lampLightContainer.update(_dataLock);
        _tertiaryDataContainer.update(_dataLock);
    }

    void calculateDistance2(const i32v3& cameraPos) {
        distance2 = getDistance2(voxelPosition, cameraPos);
    }
    
    void changeState(ChunkStates State);

    void addDependency() { chunkDependencies++; }
    void removeDependency() { chunkDependencies--; }

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

    void detectNeighbors(const std::unordered_map<i32v3, Chunk*>& chunkMap);

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
    
    static double getDistance2(const i32v3& pos, const i32v3& cameraPos);

    int numNeighbors;
    bool needsNeighbors = false; // could just use -1 numNeighbors?
    std::vector<bool> activeUpdateList;
    bool drawWater;
    bool dirty;
    int loadStatus;
    // TODO(Ben): Bitfield?
    volatile bool inLoadThread;
    volatile bool inSaveThread;
    volatile bool isAccessible;
    volatile bool queuedForMesh;

    bool queuedForPhysics;
    int meshJobCounter = 0; ///< Counts the number of mesh tasks this chunk is in

    vorb::core::IThreadPoolTask<WorkerData>* lastOwnerTask; ///< Pointer to task that is working on us

    ChunkMesh *mesh;

    std::vector <TreeData> treesToLoad;
    std::vector <PlantData> plantsToLoad;
    std::vector <GLushort> spawnerBlocks;
    i32v3 voxelPosition;  // Position relative to the voxel grid

    int numBlocks;
    int minh;
    double distance2;
    bool freeWaiting;
    bool inFrustum = false;

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

    int chunkDependencies = 0; ///< Number of chunks that depend on this chunk in other threads.

    static ui32 vboIndicesID;

    Chunk *right, *left, *front, *back, *top, *bottom;

    std::shared_ptr<ChunkGridData> chunkGridData;
    ChunkPosition3D gridPosition;

    // Thread safety functions
    inline void lock() { _dataLock.lock(); }
    inline void unlock() { _dataLock.unlock(); }
    std::mutex& getDataLock() { return _dataLock; }

    ChunkID getID() const { return id; }

private:

    /// _dataLock guards chunk data.
    /// Since only the main thread modifies data, the main thread does not
    /// need to lock when reading, only when writing. All other threads should
    /// lock when reading.
    std::mutex _dataLock; ///< Lock that guards chunk data. Try to minimize locking.

    // Keeps track of which setup list we belong to
    std::vector <Chunk*>* _chunkListPtr;

    ChunkStates _state;

    ChunkID id;
    int m_iterIndex = -1;

    //The data that defines the voxels
    vvox::SmartVoxelContainer<ui16> _blockIDContainer;
    vvox::SmartVoxelContainer<ui8> _sunlightContainer;
    vvox::SmartVoxelContainer<ui16> _lampLightContainer;
    vvox::SmartVoxelContainer<ui16> _tertiaryDataContainer;

    int _levelOfDetail; ///< Determines the LOD of the chunk, 0 being base

};

//INLINE FUNCTION DEFINITIONS
#include "Chunk.inl"

inline i32 getPositionSeed(i32 x, i32 y, i32 z) {
    return ((x & 0x7FF) << 10) |
        ((y & 0x3FF)) |
        ((z & 0x7FF) << 21);
}
inline i32 getPositionSeed(i32 x, i32 z) {
    return ((x & 0xFFFF) << 16) |
        (z & 0xFFFF);
}
