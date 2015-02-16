#pragma once
#include <condition_variable>
#include <deque>
#include <list>
#include <mutex>
#include <queue>
#include <set>
#include <thread>

#include <Vorb/Vorb.h>
#include <Vorb/FixedSizeArrayRecycler.hpp>

//TODO(Ben): Use forward decl
#include "BlockData.h"
#include "Chunk.h"
#include "ChunkIOManager.h"
#include "GameManager.h"
#include "VoxelSpaceConversions.h"
#include "VoxPool.h"
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

class ChunkSlot;
class FloraTask;
class Frustum;
class GenerateTask;
class GeneratedTreeNodes;
class PhysicsEngine;
class RenderTask;
class VoxelLightEngine;

class HeightmapGenRpcDispatcher {
public:
    HeightmapGenRpcDispatcher(SphericalTerrainGpuGenerator* generator) :
        m_generator(generator) {
        for (int i = 0; i < NUM_GENERATORS; i++) {
            m_generators[i].generator = m_generator;
        }
    }
    /// @return a new mesh on success, nullptr on failure
    bool dispatchHeightmapGen(std::shared_ptr<ChunkGridData>& cgd, const ChunkPosition3D& facePosition, float voxelRadius);
private:
    static const int NUM_GENERATORS = 512;
    int counter = 0;

    SphericalTerrainGpuGenerator* m_generator = nullptr;

    RawGenDelegate m_generators[NUM_GENERATORS];
};

// ChunkManager will keep track of all chunks and their states, and will update them.
class ChunkManager {
public:
    ChunkManager(PhysicsEngine* physicsEngine,
                 SphericalTerrainGpuGenerator* terrainGenerator,
                 const ChunkPosition2D& startGridPos, ChunkIOManager* chunkIo,
                 const f64v3& gridPosition, float planetRadius);
    ~ChunkManager();

    enum InitFlags {
        FLAT_GRASS,
        SET_Y_TO_SURFACE
    };

    /// Updates the chunks
    /// @param position: The position of the observer
    /// @param frustum: View frustum of observer
    void update(const f64v3& position, const Frustum* frustum);

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
    std::shared_ptr<ChunkGridData> getChunkGridData(const i32v2& gridPos);

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

    const std::vector<Chunk*>& getChunks() const {
        return m_chunks;
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

    /// Setters
    void setIsStationary(bool isStationary) {
        _isStationary = isStationary;
    }

    void setTerrainGenerator(SphericalTerrainGpuGenerator* generator);
    /// Getters
    PhysicsEngine* getPhysicsEngine() { return m_physicsEngine; }

private:

    /// Initializes the threadpool
    void initializeThreadPool();

    /// Gets all finished tasks from threadpool
    void processFinishedTasks();

    /// Processes a generate task that is finished
    void processFinishedGenerateTask(GenerateTask* task);

    /// Processes a flora task that is finished
    void processFinishedFloraTask(FloraTask* task);

    /// Updates all chunks that have been loaded
    void updateLoadedChunks(ui32 maxTicks);

    /// Updates all chunks that are ready to be generated
    void updateGenerateList();

    /// Adds a generate task to the threadpool
    void addGenerateTask(Chunk* chunk);

    /// Creates a chunk and any needed grid data at a given chunk position
    /// @param chunkPosition: position to create the chunk at
    /// @param relativeGridPos: the gridPosition that this chunk is relative to.
    /// @param ijOffset the ij grid offset from the relative map data. Defauts to no offset
    void makeChunkAt(const i32v3& chunkPosition, const ChunkPosition2D& relativeGridPos, const i32v2& ijOffset = i32v2(0));

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

    /// Adds a chunk to the generateList
    /// @param chunk: the chunk to add
    void addToGenerateList(Chunk* chunk);

    /// Recycles a chunk
    /// @param chunk: the chunk to recycle
    void recycleChunk(Chunk* chunk);

    /// Produces a chunk, either from recycled chunks or new
    Chunk* produceChunk();

    /// Deletes all recycled chunks
    void deleteAllChunks();

    /// Updates all chunks
    /// @param position: the observer position
    /// @param frustum: The view frustum of the observer
    void updateChunks(const f64v3& position, const Frustum* frustum);

    /// Updates the neighbors for a chunk, possibly loading new chunks
    /// @param chunk: the chunk in question
    /// @param cameraPos: camera position
    void updateChunkNeighbors(Chunk* chunk, const i32v3& cameraPos);

    /// Tries to load a chunk neighbor if it is in range
    /// @param chunk: relative chunk
    /// @param cameraPos: the camera position
    /// @param offset: the offset, must be unit length.
    void tryLoadChunkNeighbor(Chunk* chunk, const i32v3& cameraPos, const i32v3& offset);

    /// Calculates cave occlusion. This is temporarily broken /// TODO(Ben): Fix cave occlusion
    void caveOcclusion(const f64v3& ppos);
    /// Used by cave occlusion
    void recursiveFloodFill(bool left, bool right, bool front, bool back, bool top, bool bottom, Chunk* chunk);

    /// Simple debugging print
    void printOwnerList(Chunk* chunk);

    /// True when the chunk can be sent to mesh thread. Will set neighbor dependencies
    bool trySetMeshDependencies(Chunk* chunk);

    /// Removes all mesh dependencies
    void tryRemoveMeshDependencies(Chunk* chunk);

    //***** Private Variables *****

    /// Theoretical maximum number of chunks that could be in memory at one time.
    /// It is actually bigger than the maximum for safety
    i32 _csGridSize;

    /// list of chunks that are waiting for threads to finish on them
    std::vector<Chunk*> _freeWaitingChunks;
    std::vector<Chunk*> m_chunks;

    /// hashmap of chunks
    std::unordered_map<i32v3, Chunk*> m_chunkMap;

    /// Chunk Lists
    /// Stack of free chunks that have been recycled and can be used
    std::vector<Chunk*> _freeList;
    /// Stack of chunks needing setup
    std::vector<Chunk*> _setupList;
    /// Stack of chunks that need to be meshed on the threadPool
    std::vector<Chunk*> _meshList;
    /// Stack of chunks that need to be sent to the IO thread
    std::vector<Chunk*> _loadList;
    /// Stack of chunks needing generation
    std::vector<Chunk*> m_generateList;

    /// Indexed by (x,z)
    std::unordered_map<i32v2, std::shared_ptr<ChunkGridData> > _chunkGridDataMap;

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

    /// Voxel mapping data at the camera
    /// TODO(Ben): This is temporary
    ChunkPosition2D m_cameraGridPos;
    i32v2 m_prevCameraChunkPos;

    /// The threadpool for generating chunks and meshes
    vcore::ThreadPool<WorkerData> _threadPool;

    /// Dispatches asynchronous generation requests
    std::unique_ptr<HeightmapGenRpcDispatcher> heightmapGenRpcDispatcher = nullptr;

    /// Generates voxel heightmaps
    SphericalTerrainGpuGenerator* m_terrainGenerator = nullptr;

    int _numCaTasks = 0; ///< The number of CA tasks currently being processed

    VoxelLightEngine* _voxelLightEngine; ///< Used for checking top chunks for sunlight

    PhysicsEngine* m_physicsEngine = nullptr;

    ChunkIOManager* m_chunkIo = nullptr;

    float m_planetRadius = 0; ///< Radius in km

    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui16> _shortFixedSizeArrayRecycler; ///< For recycling voxel data
    vcore::FixedSizeArrayRecycler<CHUNK_SIZE, ui8> _byteFixedSizeArrayRecycler; ///< For recycling voxel data
};

