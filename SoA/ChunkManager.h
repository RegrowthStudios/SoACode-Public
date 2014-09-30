#pragma once
#include <condition_variable>
#include <deque>
#include <list>
#include <mutex>
#include <queue>
#include <set>
#include <thread>

#include "BlockData.h"
#include "Chunk.h"
#include "GameManager.h"
#include "ThreadPool.h"
#include "ChunkIOManager.h"
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
    void initialize(const f64v3& gridPosition, FaceData* playerFaceData, ui32 flags);

    void resizeGrid(const f64v3& gpos);
    void relocateChunks(const f64v3& gpos);
    void regenerateHeightMap(i32 loadType);

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

    void initializeHeightMap();
    void initializeChunks();
    void clearAllChunks(bool clearDrawing);
    void clearAll();
    void loadAllChunks(i32 loadType, const f64v3& position);
    void saveAllChunks();
    void recursiveSortChunks(std::vector<Chunk*>& v, i32 start, i32 size, i32 type);

    bool isChunkPositionInBounds(const i32v3& position) const;

    void getBlockAndChunk(const i32v3& relativePosition, Chunk** chunk, int& blockIndex) const;

    //getters
    const std::unordered_map<i32v3, int>& getChunkSlotIndexMap() const {
        return _chunkSlotIndexMap;
    }
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

    i32v3 cornerPosition;

    i32 csGridSize;

    //TODO: Implement this
    std::deque< std::deque<class CloseTerrainPatch*> > closeTerrainPatches;

    class Planet* planet;
    ThreadPool threadPool;

    bool generateOnly;

private:
    void initializeGrid(const f64v3& gpos, GLuint flags);
    void initializeMinerals();
    void updateLoadList(ui32 maxTicks);
    i32 updateSetupList(ui32 maxTicks);
    i32 updateMeshList(ui32 maxTicks, const f64v3& position);
    i32 updateGenerateList(ui32 maxTicks);
    void setupNeighbors(Chunk* chunk);
    inline void shiftX(i32 dir);
    inline void shiftY(i32 dir);
    inline void shiftZ(i32 dir);
    void calculateCornerPosition(const f64v3& centerPosition);
    void shiftHeightMap(i32 dir);
    void prepareHeightMap(HeightData heightData[CHUNK_LAYER], i32 startX, i32 startZ, i32 width, i32 height);
    void clearChunkFromLists(Chunk* chunk);
    void clearChunk(Chunk* chunk);
    void recursiveSortSetupList(std::vector<Chunk*>& v, i32 start, i32 size, i32 type);
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

    //used to reference the current world face of the player
    FaceData* _playerFace;

    ui32 _maxChunkTicks;

    std::vector<ChunkSlot> _chunkSlots[6]; //one per LOD
    std::vector<Chunk*> _threadWaitingChunks;
    std::unordered_map<i32v3, int> _chunkSlotIndexMap;

    std::vector<Chunk*> _freeList;
    std::vector<Chunk*> _setupList;
    std::vector<Chunk*> _generateList;
    std::vector<Chunk*> _meshList;
    std::vector<Chunk*> _loadList;

    queue<ChunkMeshData*> _finishedChunkMeshes;

    struct RenderTask* _mRenderTask;

    i32 _maxLoads;
    i32 _hz, _hx;

    std::vector< std::vector< HeightData*> > _heightMap; //not a 2d array like you expect. i is the ith chunk grid, j is all the data in that grid
    std::vector< std::vector<FaceData*> > _faceMap;

    i32 _poccx, _poccy, _poccz;

    f64v3 _cright, _cup, _cfront, _wpos;

    bool _physicsDisabled;
    bool _isHugeShift;
    bool _isStationary;

    ChunkDiagnostics _chunkDiagnostics;
};

