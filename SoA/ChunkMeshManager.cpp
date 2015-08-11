#include "stdafx.h"
#include "ChunkMeshManager.h"

#include "ChunkMesh.h"
#include "ChunkMeshTask.h"
#include "ChunkMesher.h"
#include "ChunkRenderer.h"
#include "SpaceSystemComponents.h"
#include "soaUtils.h"

#define MAX_UPDATES_PER_FRAME 300

ChunkMeshManager::ChunkMeshManager(vcore::ThreadPool<WorkerData>* threadPool, BlockPack* blockPack, ui32 startMeshes /*= 128*/) {
    m_meshStorage.resize(startMeshes);
    m_freeMeshes.resize(startMeshes);
    for (ui32 i = 0; i < startMeshes; i++) {
        m_freeMeshes[i] = i;
    }

    m_threadPool = threadPool;
    m_blockPack = blockPack;
    SpaceSystemAssemblages::onAddSphericalVoxelComponent += makeDelegate(*this, &ChunkMeshManager::onAddSphericalVoxelComponent);
    SpaceSystemAssemblages::onRemoveSphericalVoxelComponent += makeDelegate(*this, &ChunkMeshManager::onRemoveSphericalVoxelComponent);
}

void ChunkMeshManager::update(const f64v3& cameraPosition, bool shouldSort) {
    ChunkMeshMessage updateBuffer[MAX_UPDATES_PER_FRAME];
    size_t numUpdates;
    if (numUpdates = m_messages.try_dequeue_bulk(updateBuffer, MAX_UPDATES_PER_FRAME)) {
        for (size_t i = 0; i < numUpdates; i++) {
            processMessage(updateBuffer[i]);
        }
    }

    // TODO(Ben): This is redundant with the chunk manager! Find a way to share! (Pointer?)
    updateMeshDistances(cameraPosition);
    if (shouldSort) {
        
    }
}

void ChunkMeshManager::destroy() {
    std::vector <ChunkMesh*>().swap(m_activeChunkMeshes);
    moodycamel::ConcurrentQueue<ChunkMeshMessage>().swap(m_messages);
    std::vector <ChunkMesh::ID>().swap(m_freeMeshes);
    std::vector <ChunkMesh>().swap(m_meshStorage);
    std::unordered_map<ChunkID, ChunkMesh::ID>().swap(m_activeChunks);
}

void ChunkMeshManager::processMessage(ChunkMeshMessage& message) {
    switch (message.messageID) {
        case ChunkMeshMessageID::CREATE:
            createMesh(message);
            break;
        case ChunkMeshMessageID::DESTROY:
            destroyMesh(message);
            break;
        case ChunkMeshMessageID::UPDATE:
            updateMesh(message);
            break;
    }
}

void ChunkMeshManager::createMesh(ChunkMeshMessage& message) {
    ChunkHandle& h = message.chunk;
    // Check if we are supposed to be destroyed
    auto& it = m_pendingDestroy.find(h.getID());
    if (it != m_pendingDestroy.end()) {
        m_pendingDestroy.erase(it);
        return;
    }
    // Make sure we can even get a free mesh
    updateMeshStorage();
    // Get a free mesh
    ChunkMesh::ID id = m_freeMeshes.back();
    m_freeMeshes.pop_back();
    ChunkMesh& mesh = m_meshStorage[id];
    mesh.id = id;

    // Set the position
    mesh.position = h->m_voxelPosition;

    // Zero buffers
    memset(mesh.vbos, 0, sizeof(mesh.vbos));
    memset(mesh.vaos, 0, sizeof(mesh.vaos));
    mesh.transIndexID = 0;
    mesh.activeMeshesIndex = ACTIVE_MESH_INDEX_NONE;

    // Register chunk as active and give it a mesh
    m_activeChunks[h.getID()] = id;

    // TODO(Ben): Recycler
    ChunkMeshTask* meshTask = new ChunkMeshTask;
    meshTask->init(h, MeshTaskType::DEFAULT, m_blockPack, this);
    
    // TODO(Ben): Mesh dependencies
    //m_threadPool->addTask(meshTask);
}

void ChunkMeshManager::destroyMesh(ChunkMeshMessage& message) {
    ChunkHandle& h = message.chunk;
    // Get the mesh object
    auto& it = m_activeChunks.find(h.getID());
    // Check for rare case where destroy comes before create, just re-enqueue.
    if (it == m_activeChunks.end()) {
        m_pendingDestroy.insert(h.getID());
        return;
    }

    ChunkMesh::ID& id = it->second;
    ChunkMesh& mesh = m_meshStorage[id];

    // De-allocate buffer objects
    glDeleteBuffers(4, mesh.vbos);
    glDeleteVertexArrays(4, mesh.vaos);
    if (mesh.transIndexID) glDeleteBuffers(1, &mesh.transIndexID);

    // Remove from mesh list
    if (mesh.activeMeshesIndex != ACTIVE_MESH_INDEX_NONE) {
        m_activeChunkMeshes[mesh.activeMeshesIndex] = m_activeChunkMeshes.back();
        m_activeChunkMeshes[mesh.activeMeshesIndex]->activeMeshesIndex = mesh.activeMeshesIndex;
        m_activeChunkMeshes.pop_back();
        mesh.activeMeshesIndex = ACTIVE_MESH_INDEX_NONE;
    }
    // Release the mesh object
    m_freeMeshes.push_back(id);
    m_activeChunks.erase(it);
}

void ChunkMeshManager::updateMesh(ChunkMeshMessage& message) {   
    ChunkHandle& h = message.chunk;
    // Get the mesh object
    auto& it = m_activeChunks.find(h.getID());
    if (it == m_activeChunks.end()) {
        delete message.meshData;
        return; /// The mesh was already released, so ignore!
    }

    ChunkMesh &mesh = m_meshStorage[it->second];

    if (ChunkMesher::uploadMeshData(mesh, message.meshData)) {
        // Add to active list if its not there
        if (mesh.activeMeshesIndex == ACTIVE_MESH_INDEX_NONE) {
            mesh.activeMeshesIndex = m_activeChunkMeshes.size();
            m_activeChunkMeshes.push_back(&mesh);
        }
    } else {
        // Remove from active list
        if (mesh.activeMeshesIndex != ACTIVE_MESH_INDEX_NONE) {
            m_activeChunkMeshes[mesh.activeMeshesIndex] = m_activeChunkMeshes.back();
            m_activeChunkMeshes[mesh.activeMeshesIndex]->activeMeshesIndex = mesh.activeMeshesIndex;
            m_activeChunkMeshes.pop_back();
            mesh.activeMeshesIndex = ACTIVE_MESH_INDEX_NONE;
        }
    }

    // TODO(Ben): come on...
    delete message.meshData;
}

void ChunkMeshManager::updateMeshDistances(const f64v3& cameraPosition) {
    static const f64v3 CHUNK_DIMS(CHUNK_WIDTH);

    for (auto& mesh : m_activeChunkMeshes) { //update distances for all chunk meshes
        //calculate distance
        f64v3 closestPoint = getClosestPointOnAABB(cameraPosition, mesh->position, CHUNK_DIMS);
        // Omit sqrt for faster calculation
        mesh->distance2 = selfDot(closestPoint - cameraPosition);
    }
}

void ChunkMeshManager::updateMeshStorage() {
    if (m_freeMeshes.empty()) {
        // Need to cache all indices
        std::vector<ChunkMesh::ID> tmp(m_activeChunkMeshes.size());
        for (size_t i = 0; i < tmp.size(); i++) {
            tmp[i] = m_activeChunkMeshes[i]->id;
        }

        // Resize the storage
        ui32 i = m_meshStorage.size();
        m_meshStorage.resize((ui32)(m_meshStorage.size() * 1.5f));
        for (; i < m_meshStorage.size(); i++) {
            m_freeMeshes.push_back(i);
        }

        // Set the pointers again, since they were invalidated
        for (size_t i = 0; i < tmp.size(); i++) {
            m_activeChunkMeshes[i] = &m_meshStorage[tmp[i]];
        }
    }
}

void ChunkMeshManager::onAddSphericalVoxelComponent(Sender s, SphericalVoxelComponent& cmp, vecs::EntityID e) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < cmp.chunkGrids[i].numGenerators; j++) {
            cmp.chunkGrids[i].generators[j].onGenFinish += makeDelegate(*this, &ChunkMeshManager::onGenFinish);
        }
        cmp.chunkGrids[i].accessor.onRemove += makeDelegate(*this, &ChunkMeshManager::onAccessorRemove);
    }
}

void ChunkMeshManager::onRemoveSphericalVoxelComponent(Sender s, SphericalVoxelComponent& cmp, vecs::EntityID e) {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < cmp.chunkGrids[i].numGenerators; j++) {
            cmp.chunkGrids[i].generators[j].onGenFinish -= makeDelegate(*this, &ChunkMeshManager::onGenFinish);
        }
        cmp.chunkGrids[i].accessor.onRemove -= makeDelegate(*this, &ChunkMeshManager::onAccessorRemove);
    }
}

void ChunkMeshManager::onGenFinish(Sender s, ChunkHandle chunk, ChunkGenLevel gen) {
    // Can be meshed.
    if (gen == GEN_DONE) {
        // Create message
        ChunkMeshMessage msg;
        chunk.acquire();
        msg.chunk = chunk;
        msg.messageID = ChunkMeshMessageID::CREATE;
        m_messages.enqueue(msg);
    }
}

void ChunkMeshManager::onAccessorRemove(Sender s, ChunkHandle chunk) {
    // TODO(Ben): Implement
}