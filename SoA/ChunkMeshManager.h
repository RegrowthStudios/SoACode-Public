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

enum class ChunkMeshMessageID { CREATE, UPDATE, DESTROY };

struct ChunkMeshMessage {
    ChunkHandle chunk;
    ChunkMeshMessageID messageID;
    ChunkMeshData* meshData = nullptr;
};

class ChunkMeshManager {
public:
    ChunkMeshManager(vcore::ThreadPool<WorkerData>* threadPool, BlockPack* blockPack, ui32 startMeshes = 128);
    /// Updates the meshManager, uploading any needed meshes
    void update(const f64v3& cameraPosition, bool shouldSort);
    /// Adds a mesh for updating
    void sendMessage(const ChunkMeshMessage& message) { m_messages.enqueue(message); }
    /// Destroys all meshes
    void destroy();

    const std::vector <ChunkMesh *>& getChunkMeshes() { return m_activeChunkMeshes; }

private:
    VORB_NON_COPYABLE(ChunkMeshManager);

    void processMessage(ChunkMeshMessage& message);

    void createMesh(ChunkMeshMessage& message);

    ChunkMeshTask* trySetMeshDependencies(ChunkHandle chunk);

    void destroyMesh(ChunkMeshMessage& message);

    void disposeChunkMesh(Chunk* chunk);

    /// Uploads a mesh and adds to list if needed
    void updateMesh(ChunkMeshMessage& message);

    void updateMeshDistances(const f64v3& cameraPosition);

    void updateMeshStorage();
    /************************************************************************/
    /* Event Handlers                                                       */
    /************************************************************************/
    void onAddSphericalVoxelComponent(Sender s, SphericalVoxelComponent& cmp, vecs::EntityID e);
    void onRemoveSphericalVoxelComponent(Sender s, SphericalVoxelComponent& cmp, vecs::EntityID e);
    void onGenFinish(Sender s, ChunkHandle chunk, ChunkGenLevel gen);
    void onAccessorRemove(Sender s, ChunkHandle chunk);

    /************************************************************************/
    /* Members                                                              */
    /************************************************************************/
    std::vector <ChunkMesh*> m_activeChunkMeshes; ///< Meshes that should be drawn
    moodycamel::ConcurrentQueue<ChunkMeshMessage> m_messages; ///< Lock-free queue of messages

    BlockPack* m_blockPack = nullptr;
    vcore::ThreadPool<WorkerData>* m_threadPool = nullptr;

    std::vector <ChunkMesh::ID> m_freeMeshes; ///< Stack of free mesh indices
    std::vector <ChunkMesh> m_meshStorage; ///< Cache friendly mesh object storage
    std::unordered_map<ChunkID, ChunkMesh::ID> m_activeChunks; ///< Stores chunk IDs that have meshes
    std::set<ChunkID> m_pendingDestroy;
};

#endif // ChunkMeshManager_h__
