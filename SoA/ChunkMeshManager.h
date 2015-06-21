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
#include "NChunk.h"
#include "ChunkMesh.h"
#include <mutex>

enum class ChunkMeshMessageID { CREATE, UPDATE, DESTROY };

struct ChunkMeshMessage {
    ChunkID chunkID;
    ChunkMeshMessageID messageID;
    void* data;
};

class ChunkMeshManager {
public:
    ChunkMeshManager(ui32 startMeshes = 128);
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

    void destroyMesh(ChunkMeshMessage& message);
    /// Uploads a mesh and adds to list if needed
    void updateMesh(ChunkMeshMessage& message);

    void updateMeshDistances(const f64v3& cameraPosition);

    void updateMeshStorage();

    std::vector <ChunkMesh*> m_activeChunkMeshes; ///< Meshes that should be drawn
    moodycamel::ConcurrentQueue<ChunkMeshMessage> m_messages; ///< Lock-free queue of messages

    std::vector <ChunkMesh::ID> m_freeMeshes; ///< Stack of free mesh indices
    std::vector <ChunkMesh> m_meshStorage; ///< Cache friendly mesh object storage
    std::unordered_map<ChunkID, ChunkMesh::ID> m_activeChunks; ///< Stores chunk IDs that have meshes
};

#endif // ChunkMeshManager_h__
