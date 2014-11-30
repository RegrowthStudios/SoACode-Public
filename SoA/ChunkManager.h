#pragma once
#include <condition_variable>
#include <deque>
#include <list>
#include <mutex>
#include <queue>
#include <set>
#include <thread>

#include "Vorb.h"

#include "BlockData.h"
#include "Chunk.h"
#include "ChunkIOManager.h"
#include "FixedSizeArrayRecycler.hpp"
#include "GameManager.h"
#include "IVoxelMapper.h"
#include "ThreadPool.h"
#include "WorldStructs.h"

const i32 lodStep = 1;

extern ui32 strtTimer;

// Message used when placing blocks
class PlaceBlocksMessage {

    PlaceBlocksMessage(Item *equipped) : equippedItem(equipped) {}

    Item* equippedItem;
};

class ChunkDiagnostics {
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

class Camera;
class ChunkSlot;
class FloraTask;
class GenerateTask;
class GeneratedTreeNodes;
class RenderTask;
class VoxelLightEngine;

// ChunkManager will keep track of all chunks and their states, and will update them.
class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    enum InitFlags {
        FLAT_GRASS,
        SET_Y_TO_SURFACE
    };

    /// Initializes the grid at the surface and returns the Y value
    /// @param gridPosition: the floating point starting grid position.
    /// @param voxelMapper: The chosen voxel mapping scheme
    /// @param flags: bitwise combination of ChunkManager::InitFlags
    void initialize(const f64v3& gridPosition, vvox::IVoxelMapper* voxelMapper, vvox::VoxelMapData* startingMapData, ui32 flags);

    /// Updates the chunks
    /// @param camera: The camera that is rendering the voxels
    void update(const Camera* camera);

    /// Gets the 8 closest chunks to a point
    /// @param coord: the position in question
    /// @param chunks: a pointer to a Chunk[8] that will be filled by the function
    void getClosestChunks(f64v3& coord, class Chunk** chunks);

    /// Gets the block that is intersecting with a ray
    /// @param dir: the ray direction
    /// @param pos: the ray start position
    /// Returns the block ID of the selected block
    i32 getBlockFromDir(f64v3& dir, f64v3& pos);

    /// Gets the height data at position
    /// @param posX: the X position
    /// @param posY: the Y position
    /// @param hd: the HeightData result to be stored
    /// Returns 1 on fail, 0 on success
    i32 getPositionHeightData(i32 posX, i32 posZ, HeightData& hd);

    /// Gets a chunk at a given floating point position
    /// @param position: The position
    /// Returns nullptr if no chunk is found, otherwise returns the chunk
    Chunk* getChunk(const f64v3& position);

    /// Gets a chunk at a given chunk grid position
    /// @param chunkPos: The chunk grid position, units are of chunk length
    /// Returns nullptr if no chunk is found, otherwise returns the chunk
    Chunk* getChunk(const i32v3& chunkPos);

    /// Gets a const chunk at a given chunk grid position
    /// @param chunkPos: The chunk grid position, units are of chunk length
    /// Returns nullptr if no chunk is found, otherwise returns the chunk
    const Chunk* getChunk(const i32v3& chunkPos) const;

    /// Gets the grid data at a given chunk grid position
    /// @param gridPos: The chunk grid position, units are of chunk length
    /// Returns nullptr if no chunkGridData is found, otherwise returns a chunkGridData
    ChunkGridData* getChunkGridData(const i32v2& gridPos);

    /// Clears everything, freeing all memory. Should be called when ending a game
    void destroy();

    /// Saves all chunks that are dirty
    void saveAllChunks();

    /// Gets a block index and chunk at a given voxel position
    /// @param blockPosition: the voxel position
    /// @param chunk: a pointer to a chunk pointer to be stored
    /// @param blockIndex: the resulting block index to be stored
    void getBlockAndChunk(const i32v3& blockPosition, Chunk** chunk, int& blockIndex);

    /// Returns a chunk position given a block position
    /// @param blockPosition: the block position
    inline i32v3 getChunkPosition(const i32v3& blockPosition) const {
        return i32v3(fastFloor(blockPosition.x / (f64)CHUNK_WIDTH),
                     fastFloor(blockPosition.y / (f64)CHUNK_WIDTH),
                     fastFloor(blockPosition.z / (f64)CHUNK_WIDTH));
    }
    /// Returns a chunk position given a floating point grid position
    /// @param gridPosition: the block position
    inline i32v3 getChunkPosition(const f32v3& blockPosition) const {
        return i32v3(fastFloor(blockPosition.x / (f64)CHUNK_WIDTH),
                     fastFloor(blockPosition.y / (f64)CHUNK_WIDTH),
                     fastFloor(blockPosition.z / (f64)CHUNK_WIDTH));
    }
    /// Returns a chunk position given a floating point grid position
    /// @param gridPosition: the block position
    inline i32v3 getChunkPosition(const f64v3& blockPosition) const {
        return i32v3(fastFloor(blockPosition.x / (f64)CHUNK_WIDTH),
                     fastFloor(blockPosition.y / (f64)CHUNK_WIDTH),
                     fastFloor(blockPosition.z / (f64)CHUNK_WIDTH));
    }

    /// Returns the current chunk diagnostics
    const ChunkDiagnostics& getChunkDiagnostics() const {
        return _chunkDiagnostics;
    }

    const std::vector<ChunkSlot>& getChunkSlots(int levelOfDetail) const {
        return _chunkSlots[levelOfDetail];
    }

    /// Forces a remesh of all chunks
    void remeshAllChunks();

    /// Gets a list of IDs from a ray query
    /// @param start: starting ray position
    /// @param end: ending ray position
    /// Returns an array of IDs that should be freed by the caller.
    const i16* getIDQuery(const i32v3& start, const i32v3& end) const;

    /// Updates the cellular automata physics
    void updateCaPhysics();

    /// ***** Public variables ******

    /// TODO(Ben): Implement this
    std::deque< std::deque<class CloseTerrainPatch*> > closeTerrainPatches;

    /// Pointer to the current planet /// TODO(Ben): move this somewhere else?
    class Planet* planet;

    /// Setters
    void setIsStationary(bool isStationary) {
        _isStationary = isStationary;
    }

private:

    /// Initializes the threadpool
    void initializeThreadPool();

    /// Gets all finished tasks from threadpool
    void processFinishedTasks();

    /// Processes a generate task that is finished
    void processFinishedGenerateTask(GenerateTask* task);

    /// Processes a render task that is finished
    void processFinishedRenderTask(RenderTask* task);

    /// Processes a flora task that is finished
    void processFinishedFloraTask(FloraTask* task);

    /// Updates all chunks that have been loaded
    void updateLoadedChunks(ui32 maxTicks);

    /// Creates a chunk and any needed grid data at a given chunk position
    /// @param chunkPosition: position to create the chunk at
    /// @param relativeMapData: the voxelMapData that this chunk is relative to.
    /// @param ijOffset the ij grid offset from the relative map data. Defauts to no offset
    void makeChunkAt(const i32v3& chunkPosition, const vvox::VoxelMapData* relativeMapData, const i32v2& ijOffset = i32v2(0));

    /// Initializes minerals. This function is temporary.
    void initializeMinerals();

    /// Updates the load list
    /// @param maxTicks: maximum time the function is allowed
    void updateLoadList(ui32 maxTicks);

    /// Updates the setup list
    /// @param maxTicks: maximum time the function is allowed
    i32 updateSetupList(ui32 maxTicks);

    /// Updates the mesh list
    /// @param maxTicks: maximum time the function is allowed
    i32 updateMeshList(ui32 maxTicks);

    /// Updates the treesToPlace list
    /// @param maxTicks: maximum time the function is allowed
    void updateTreesToPlace(ui32 maxTicks);

    /// Places a batch of tree nodes
    /// @param nodes: the nodes to place
    void placeTreeNodes(GeneratedTreeNodes* nodes);

    /// Setups any chunk neighbor connections
    /// @param chunk: the chunk to connect
    void setupNeighbors(Chunk* chunk);

    /// Frees a chunk from the world. 
    /// The chunk may be recycled, or it may need to wait for some threads
    /// to finish processing on it.
    /// @param chunk: the chunk to free
    void freeChunk(Chunk* chunk);

    /// Adds a chunk to the setupList
    /// @param chunk: the chunk to add
    void addToSetupList(Chunk* chunk);

    /// Adds a chunk to the loadList
    /// @param chunk: the chunk to add
    void addToLoadList(Chunk* chunk);

    /// Adds a chunk to the meshList
    /// @param chunk: the chunk to add
    void addToMeshList(Chunk* chunk);

    /// Recycles a chunk
    /// @param chunk: the chunk to recycle
    void recycleChunk(Chunk* chunk);

    /// Produces a chunk, either from recycled chunks or new
    Chunk* produceChunk();

    /// Deletes all recycled chunks
    void deleteAllChunks();

    /// Updates all chunks
    /// @param position: the camera position
    void updateChunks(const Camera* cameran);

    /// Updates the neighbors for a chunk slot, possible loading new chunks
    /// @param cs: the chunkslot in question
    /// @param cameraPos: camera position
    void updateChunkslotNeighbors(ChunkSlot* cs, const i32v3& cameraPos);

    /// Tries to load a chunk slot neighbor if it is in range
    /// @param cs: relative chunk slot
    /// @param cameraPos: the camera position
    /// @param offset: the offset, must be unit length.
    ChunkSlot* tryLoadChunkslotNeighbor(ChunkSlot* cs, const i32v3& cameraPos, const i32v3& offset);

    /// Calculates cave occlusion. This is temporarily broken /// TODO(Ben): Fix cave occlusion
    void caveOcclusion(const f64v3& ppos);
    /// Used by cave occlusion
    void recursiveFloodFill(bool left, bool right, bool front, bool back, bool top, bool bottom, Chunk* chunk);

    //***** Private Variables *****

    /// Theoretical maximum number of chunks that could be in memory at one time.
    /// It is actually bigger than the maximum for safety
    i32 _csGridSize;

    /// All chunks slots. Right now only the first is used,
    /// but ideally there is one vector per LOD level for recombination
    /// TODO(Ben): Implement recombination
    std::vector<ChunkSlot> _chunkSlots[6]; //one per LOD

    /// list of chunks that are waiting for threads to finish on them
    std::vector<Chunk*> _freeWaitingChunks;

    /// hashmap of chunk slots
    std::unordered_map<i32v3, ChunkSlot*> _chunkSlotMap;

    /// Chunk Lists
    /// Stack of free chunks that have been recycled and can be used
    std::vector<Chunk*> _freeList;
    /// Stack of chunks needing setup
    std::vector<Chunk*> _setupList;
    /// Stack of chunks that need to be meshed on the threadPool
    std::vector<Chunk*> _meshList;
    /// Stack of chunks that need to be sent to the IO thread
    std::vector<Chunk*> _loadList;

    /// Indexed by (x,z)
    std::unordered_map<i32v2, ChunkGridData*> _chunkGridDataMap;

    std::vector<GeneratedTreeNodes*> _treesToPlace; ///< List of tree batches we need to place

    /// Max size of the tasks vectors
    #define MAX_CACHED_TASKS 50
    std::vector<RenderTask*> _freeRenderTasks; ///< For recycling render tasks
    std::vector<GenerateTask*> _freeGenerateTasks; ///< For recycling generateTasks

    /// Used by cave occlusion
    i32 _poccx, _poccy, _poccz;

    /// True if the chunk world should never move
    bool _isStationary;

    /// Stores information about chunks
    ChunkDiagnostics _chunkDiagnostics;

    /// The current voxel mapping scheme
    vvox::IVoxelMapper* _voxelMapper;

    /// Voxel mapping data at the camera
    vvox::VoxelMapData* _cameraVoxelMapData;

    /// The threadpool for generating chunks and meshes
    vcore::ThreadPool _threadPool;

    int _numCaTasks = 0; ///< The number of CA tasks currently being processed

    VoxelLightEngine* _voxelLightEngine; ///< Used for checking top chunks for sunlight

    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16> _shortFixedSizeArrayRecycler; ///< For recycling voxel data
    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8> _byteFixedSizeArrayRecycler; ///< For recycling voxel data
};

