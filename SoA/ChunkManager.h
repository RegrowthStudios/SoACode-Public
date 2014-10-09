#pragma once
#include <condition_variable>
#include <deque>
#include <list>
#include <mutex>
#include <queue>
#include <set>
#include <thread>

#include <boost/circular_buffer.hpp>

#include "Vorb.h"

#include "BlockData.h"
#include "Chunk.h"
#include "GameManager.h"
#include "ThreadPool.h"
#include "ChunkIOManager.h"
#include "IVoxelMapper.h"
#include "WorldStructs.h"

const i32 lodStep = 1;

extern ui32 strtTimer;

struct PlaceBlocksMessage {

    PlaceBlocksMessage(Item *equipped) : equippedItem(equipped) {}

    Item* equippedItem;
};

struct ChunkDiagnostics {
public:
    // Number Of Chunks That Have Been Allocated During The Program's Life
    i32 totalAllocated;

    // Number Of Chunks That Have Been Freed During The Program's Life
    i32 totalFreed;

    // Number Of Chunks Currently Residing In Memory
    i32 numCurrentlyAllocated;

    // Number Of Chunks Awaiting To Be Reused
    i32 numAwaitingReuse;
};

class ChunkSlot;

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    i32 getPlayerStartY();

    enum InitFlags {
        FLAT_GRASS,
        SET_Y_TO_SURFACE
    };
    // initializes the grid at the surface and returns the Y value
    void initialize(const f64v3& gridPosition, vvoxel::IVoxelMapper* voxelMapper, vvoxel::VoxelMapData* startingMapData, ui32 flags);

    void update(const f64v3& position, const f64v3& viewDir);
    i32 getClosestChunks(f64v3& coord, class Chunk** chunks);
    void drawChunkLines(glm::mat4& VP, const f64v3& position);
    i32 getBlockFromDir(f64v3& dir, f64v3& pos);
    void updateLoadedChunks();
    void uploadFinishedMeshes();
    void clearChunkData();
    i32 getPositionHeightData(i32 posX, i32 posZ, HeightData& hd);
    Chunk* getChunk(const f64v3& position);
    Chunk* getChunk(const i32v3& worldPos);
    const Chunk* getChunk(const i32v3& worldPos) const;
    ChunkGridData* getChunkGridData(const i32v2& gridPos);

    void initializeChunks(const f64v3& gridPosition);
    void clearAllChunks(bool clearDrawing);
    void clearAll();
    void saveAllChunks();
    void recursiveSortChunks(boost::circular_buffer<Chunk*>& v, i32 start, i32 size, i32 type);

    bool isChunkPositionInBounds(const i32v3& position) const;

    void getBlockAndChunk(const i32v3& relativePosition, Chunk** chunk, int& blockIndex) const;

    const vector<ChunkSlot>* getChunkSlots() const {
        return _chunkSlots;
    }
    const ChunkDiagnostics& getChunkDiagnostics() const {
        return _chunkDiagnostics;
    }

    void remeshAllChunks();

    //setters
    void setIsStationary(bool isStationary) {
        _isStationary = isStationary;
    }

    const i16* getIDQuery(const i32v3& start, const i32v3& end) const;

    i32 csGridSize;

    //TODO: Implement this
    std::deque< std::deque<class CloseTerrainPatch*> > closeTerrainPatches;

    class Planet* planet;
    ThreadPool threadPool;

    bool generateOnly;

    static i32v3 getChunkPosition(const f64v3& position);

private:

    void initializeMinerals();
    void updateLoadList(ui32 maxTicks);
    i32 updateSetupList(ui32 maxTicks);
    i32 updateMeshList(ui32 maxTicks, const f64v3& position);
    i32 updateGenerateList(ui32 maxTicks);
    void setupNeighbors(Chunk* chunk);
    void prepareHeightMap(HeightData heightData[CHUNK_LAYER]);
    void clearChunkFromLists(Chunk* chunk);
    void clearChunk(Chunk* chunk);
    void recursiveSortSetupList(boost::circular_buffer<Chunk*>& v, i32 start, i32 size, i32 type);
    void caveOcclusion(const f64v3& ppos);
    void recursiveFloodFill(bool left, bool right, bool front, bool back, bool top, bool bottom, Chunk* chunk);
    void removeFromSetupList(Chunk* ch);
    void freeChunk(Chunk* chunk);
    void addToSetupList(Chunk* chunk);
    void addToLoadList(Chunk* chunk);
    void addToGenerateList(Chunk* chunk);
    void addToMeshList(Chunk* chunk);
    void recycleChunk(Chunk* chunk);
    Chunk* produceChunk();
    void deleteAllChunks();

    void updateChunks(const f64v3& position);
    void updateChunkslotNeighbors(ChunkSlot* cs, const i32v3& cameraPos);
    ChunkSlot* tryLoadChunkslotNeighbor(ChunkSlot* cs, const i32v3& cameraPos, const i32v3& offset);

    ui32 _maxChunkTicks;

   // int _chunkSlotsSizes[6];
    std::vector<ChunkSlot> _chunkSlots[6]; //one per LOD
    std::vector<Chunk*> _threadWaitingChunks;
    std::unordered_map<i32v3, ChunkSlot*> _chunkSlotMap;

    boost::circular_buffer<Chunk*> _freeList;
    boost::circular_buffer<Chunk*> _setupList;
    boost::circular_buffer<Chunk*> _meshList;
    boost::circular_buffer<Chunk*> _loadList;
    boost::circular_buffer<Chunk*> _generateList;

    queue<ChunkMeshData*> _finishedChunkMeshes;

    struct RenderTask* _mRenderTask;

    i32 _maxLoads;

    //Indexed by (x,z)
    std::unordered_map<i32v2, ChunkGridData*> _chunkGridDataMap;

    i32 _poccx, _poccy, _poccz;

    f64v3 _cright, _cup, _cfront, _wpos;

    bool _physicsDisabled;
    bool _isStationary;

    ChunkDiagnostics _chunkDiagnostics;

    vvoxel::IVoxelMapper* _voxelMapper;
};

