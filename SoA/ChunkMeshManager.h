///
/// ChunkMeshManager.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 26 Feb 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Handles the updating and deallocation of
/// chunk meshes.
///

#pragma once

#ifndef ChunkMeshManager_h__
#define ChunkMeshManager_h__

#include "concurrentqueue.h"
#include "Chunk.h"
#include "ChunkMesh.h"
#include "SpaceSystemAssemblages.h"
#include <mutex>

struct ChunkMeshUpdateMessage {
    ChunkID chunkID;
    ChunkMeshData* meshData = nullptr;
};

class ChunkMeshManager {
public:
    ChunkMeshManager(vcore::ThreadPool<WorkerData>* threadPool, BlockPack* blockPack);
    /// Updates the meshManager, uploading any needed meshes
    void update(const f64v3& cameraPosition, bool shouldSort);
    /// Adds a mesh for updating
    void sendMessage(const ChunkMeshUpdateMessage& message) { m_messages.enqueue(message); }
    /// Destroys all meshes
    void destroy();

    // Be sure to lock lckActiveChunkMeshes
    const std::vector <ChunkMesh*>& getChunkMeshes() { return m_activeChunkMeshes; }
    std::mutex lckActiveChunkMeshes;
private:
    VORB_NON_COPYABLE(ChunkMeshManager);

    ChunkMesh* createMesh(ChunkHandle& h);

    ChunkMeshTask* createMeshTask(ChunkHandle& chunk);

    void disposeMesh(ChunkMesh* mesh);

    /// Uploads a mesh and adds to list if needed
    void updateMesh(ChunkMeshUpdateMessage& message);

    void updateMeshDistances(const f64v3& cameraPosition);

    /************************************************************************/
    /* Event Handlers                                                       */
    /************************************************************************/
    void onAddSphericalVoxelComponent(Sender s, SphericalVoxelComponent& cmp, vecs::EntityID e);
    void onRemoveSphericalVoxelComponent(Sender s, SphericalVoxelComponent& cmp, vecs::EntityID e);
    void onGenFinish(Sender s, ChunkHandle& chunk, ChunkGenLevel gen);
    void onNeighborsAcquire(Sender s, ChunkHandle& chunk);
    void onNeighborsRelease(Sender s, ChunkHandle& chunk);

    /************************************************************************/
    /* Members                                                              */
    /************************************************************************/
    std::vector<ChunkMesh*> m_activeChunkMeshes; ///< Meshes that should be drawn
    moodycamel::ConcurrentQueue<ChunkMeshUpdateMessage> m_messages; ///< Lock-free queue of messages
   
    BlockPack* m_blockPack = nullptr;
    vcore::ThreadPool<WorkerData>* m_threadPool = nullptr;

    std::mutex m_lckMeshRecycler;
    PtrRecycler<ChunkMesh> m_meshRecycler;
    std::mutex m_lckActiveChunks;
    std::unordered_map<ChunkID, ChunkMesh*> m_activeChunks; ///< Stores chunk IDs that have meshes
};

#endif // ChunkMeshManager_h__
