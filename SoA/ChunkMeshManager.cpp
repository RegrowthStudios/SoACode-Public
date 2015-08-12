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

    // TODO(Ben): Race conditions...
    // Check for mesh updates
    for (auto& m : m_activeChunkMeshes) {
        if (m->updateVersion < m->chunk->updateVersion) {
            ChunkMeshTask* task = trySetMeshDependencies(m->chunk);
            if (task) {
                m->updateVersion = m->chunk->updateVersion;
                m_threadPool->addTask(task);
                m->chunk->remeshFlags = 0;
            }
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
    mesh.chunk = h;

    // Set the position
    mesh.position = h->m_voxelPosition;

    // Zero buffers
    memset(mesh.vbos, 0, sizeof(mesh.vbos));
    memset(mesh.vaos, 0, sizeof(mesh.vaos));
    mesh.transIndexID = 0;
    mesh.activeMeshesIndex = ACTIVE_MESH_INDEX_NONE;

    // Register chunk as active and give it a mesh
    m_activeChunks[h.getID()] = id;
    
    // TODO(Ben): Mesh dependencies
    if (h->numBlocks <= 0) {
        h->remeshFlags = 0;
        return;
    }
}

ChunkMeshTask* ChunkMeshManager::trySetMeshDependencies(ChunkHandle chunk) {

    // TODO(Ben): This could be optimized a bit
    ChunkHandle& left = chunk->left;
    ChunkHandle& right = chunk->right;
    ChunkHandle& bottom = chunk->bottom;
    ChunkHandle& top = chunk->top;
    ChunkHandle& back = chunk->back;
    ChunkHandle& front = chunk->front;

    //// Check that neighbors are loaded
    if (!left->isAccessible || !right->isAccessible ||
        !front->isAccessible || !back->isAccessible ||
        !top->isAccessible || !bottom->isAccessible) return nullptr;

    // Left half
    if (!left->back.isAquired() || !left->back->isAccessible) return nullptr;
    if (!left->front.isAquired() || !left->front->isAccessible) return nullptr;

    ChunkHandle leftTop = left->top;
    ChunkHandle leftBot = left->bottom;
    if (!leftTop.isAquired() || !leftTop->isAccessible) return nullptr;
    if (!leftBot.isAquired() || !leftBot->isAccessible) return nullptr;
    if (!leftTop->back.isAquired() || !leftTop->back->isAccessible) return nullptr;
    if (!leftTop->front.isAquired() || !leftTop->front->isAccessible) return nullptr;
    if (!leftBot->back.isAquired() || !leftBot->back->isAccessible) return nullptr;
    if (!leftBot->front.isAquired() || !leftBot->front->isAccessible) return nullptr;

    // right half
    if (!right->back.isAquired() || !right->back->isAccessible) return nullptr;
    if (!right->front.isAquired() || !right->front->isAccessible) return nullptr;

    ChunkHandle rightTop = right->top;
    ChunkHandle rightBot = right->bottom;
    if (!rightTop.isAquired() || !rightTop->isAccessible) return nullptr;
    if (!rightBot.isAquired() || !rightBot->isAccessible) return nullptr;
    if (!rightTop->back.isAquired() || !rightTop->back->isAccessible) return nullptr;
    if (!rightTop->front.isAquired() || !rightTop->front->isAccessible) return nullptr;
    if (!rightBot->back.isAquired() || !rightBot->back->isAccessible) return nullptr;
    if (!rightBot->front.isAquired() || !rightBot->front->isAccessible) return nullptr;

    if (!top->back.isAquired() || !top->back->isAccessible) return nullptr;
    if (!top->front.isAquired() || !top->front->isAccessible) return nullptr;

    if (!bottom->back.isAquired() || !bottom->back->isAccessible) return nullptr;
    if (!bottom->front.isAquired() || !bottom->front->isAccessible) return nullptr;

    // TODO(Ben): Recycler
    ChunkMeshTask* meshTask = new ChunkMeshTask;
    meshTask->init(chunk, MeshTaskType::DEFAULT, m_blockPack, this);

    // Set dependencies
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT] = left.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT] = right.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_FRONT] = chunk->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_BACK] = chunk->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_TOP] = chunk->top.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_BOT] = chunk->bottom.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_BACK] = left->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_FRONT] = left->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_TOP] = leftTop.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_BOT] = leftBot.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_TOP_BACK] = leftTop->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_TOP_FRONT] = leftTop->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_BOT_BACK] = leftBot->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT_BOT_FRONT] = leftBot->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_BACK] = right->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_FRONT] = right->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_TOP] = rightTop.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_BOT] = rightBot.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_TOP_BACK] = rightTop->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_TOP_FRONT] = rightTop->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_BOT_BACK] = rightBot->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT_BOT_FRONT] = rightBot->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_TOP_BACK] = top->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_TOP_FRONT] = top->front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_BOT_BACK] = bottom->back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_BOT_FRONT] = bottom->front.acquire();

    return meshTask;
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


void ChunkMeshManager::disposeChunkMesh(Chunk* chunk) {
    // delete the mesh!
    //if (chunk->hasCreatedMesh) {
    //    ChunkMeshMessage msg;
    //    msg.chunkID = chunk->getID();
    //    msg.messageID = ChunkMeshMessageID::DESTROY;
    //   // m_cmp->chunkMeshManager->sendMessage(msg);
    //    chunk->hasCreatedMesh = false;
    //    chunk->remeshFlags |= 1;
    //}
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
            mesh.updateVersion = 0;
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

void ChunkMeshManager::onGenFinish(Sender s, ChunkHandle& chunk, ChunkGenLevel gen) {
    // Can be meshed.
    if (gen == GEN_DONE) {
        // Create message
        ChunkMeshMessage msg;
        msg.chunk = chunk.acquire();
        msg.messageID = ChunkMeshMessageID::CREATE;
        m_messages.enqueue(msg);
    }
}

void ChunkMeshManager::onAccessorRemove(Sender s, ChunkHandle& chunk) {
    chunk->updateVersion = 0;
    // Destroy message
    ChunkMeshMessage msg;
    msg.chunk = chunk;
    msg.messageID = ChunkMeshMessageID::DESTROY;
    m_messages.enqueue(msg);
}
