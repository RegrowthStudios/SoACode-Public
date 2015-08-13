#include "stdafx.h"
#include "ChunkMeshManager.h"

#include "ChunkMesh.h"
#include "ChunkMeshTask.h"
#include "ChunkMesher.h"
#include "ChunkRenderer.h"
#include "SpaceSystemComponents.h"
#include "soaUtils.h"

#define MAX_UPDATES_PER_FRAME 300

ChunkMeshManager::ChunkMeshManager(vcore::ThreadPool<WorkerData>* threadPool, BlockPack* blockPack) {
    m_threadPool = threadPool;
    m_blockPack = blockPack;
    SpaceSystemAssemblages::onAddSphericalVoxelComponent += makeDelegate(*this, &ChunkMeshManager::onAddSphericalVoxelComponent);
    SpaceSystemAssemblages::onRemoveSphericalVoxelComponent += makeDelegate(*this, &ChunkMeshManager::onRemoveSphericalVoxelComponent);
}

void ChunkMeshManager::update(const f64v3& cameraPosition, bool shouldSort) {
    ChunkMeshUpdateMessage updateBuffer[MAX_UPDATES_PER_FRAME];
    size_t numUpdates;
    if (numUpdates = m_messages.try_dequeue_bulk(updateBuffer, MAX_UPDATES_PER_FRAME)) {
        for (size_t i = 0; i < numUpdates; i++) {
            updateMesh(updateBuffer[i]);
        }
    }

    // TODO(Ben): This is redundant with the chunk manager! Find a way to share! (Pointer?)
    updateMeshDistances(cameraPosition);
    if (shouldSort) {
        
    }
}

void ChunkMeshManager::destroy() {
    std::vector <ChunkMesh*>().swap(m_activeChunkMeshes);
    moodycamel::ConcurrentQueue<ChunkMeshUpdateMessage>().swap(m_messages);
    std::unordered_map<ChunkID, ChunkMesh*>().swap(m_activeChunks);
}

ChunkMesh* ChunkMeshManager::createMesh(ChunkHandle& h) {
    ChunkMesh* mesh;
    { // Get a free mesh
        std::lock_guard<std::mutex> l(m_lckMeshRecycler);
        mesh = m_meshRecycler.create();
    }
    mesh->id = h.getID();

    // Set the position
    mesh->position = h->m_voxelPosition;

    // Zero buffers
    memset(mesh->vbos, 0, sizeof(mesh->vbos));
    memset(mesh->vaos, 0, sizeof(mesh->vaos));
    mesh->transIndexID = 0;
    mesh->activeMeshesIndex = ACTIVE_MESH_INDEX_NONE;

    { // Register chunk as active and give it a mesh
        std::lock_guard<std::mutex> l(m_lckActiveChunks);
        m_activeChunks[h.getID()] = mesh;
    }
    
    // TODO(Ben): Mesh dependencies
    if (h->numBlocks <= 0) {
        h->remeshFlags = 0;
    }

    return mesh;
}

ChunkMeshTask* ChunkMeshManager::trySetMeshDependencies(ChunkHandle chunk) {
    // TODO(Ben): Race conditions galore.

    // TODO(Ben): This could be optimized a bit
    ChunkHandle& left = chunk->left;
    ChunkHandle& right = chunk->right;
    ChunkHandle& bottom = chunk->bottom;
    ChunkHandle& top = chunk->top;
    ChunkHandle& back = chunk->back;
    ChunkHandle& front = chunk->front;

    //// Check that neighbors are loaded
    if (!left.isAquired() || !left->isAccessible ||
        !right.isAquired() || !right->isAccessible ||
        !front.isAquired() || !front->isAccessible ||
        !back.isAquired() || !back->isAccessible ||
        !top.isAquired() || !top->isAccessible ||
        !bottom.isAquired() || !bottom->isAccessible) return nullptr;

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

void ChunkMeshManager::disposeMesh(ChunkMesh* mesh) {
    // De-allocate buffer objects
    glDeleteBuffers(4, mesh->vbos);
    glDeleteVertexArrays(4, mesh->vaos);
    if (mesh->transIndexID) glDeleteBuffers(1, &mesh->transIndexID);

    { // Remove from mesh list
        std::lock_guard<std::mutex> l(lckActiveChunkMeshes);
        if (mesh->activeMeshesIndex != ACTIVE_MESH_INDEX_NONE) {
            m_activeChunkMeshes[mesh->activeMeshesIndex] = m_activeChunkMeshes.back();
            m_activeChunkMeshes[mesh->activeMeshesIndex]->activeMeshesIndex = mesh->activeMeshesIndex;
            m_activeChunkMeshes.pop_back();
            mesh->activeMeshesIndex = ACTIVE_MESH_INDEX_NONE;
        }
    }
    
    { // Release the mesh
        std::lock_guard<std::mutex> l(m_lckMeshRecycler);
        m_meshRecycler.recycle(mesh);
    }
}

void ChunkMeshManager::updateMesh(ChunkMeshUpdateMessage& message) {
    ChunkMesh *mesh;
    { // Get the mesh object
        std::lock_guard<std::mutex> l(m_lckActiveChunks);
        auto& it = m_activeChunks.find(message.chunkID);
        if (it == m_activeChunks.end()) {
            delete message.meshData;
            m_lckActiveChunks.unlock();
            return; /// The mesh was already released, so ignore!
        }
        mesh = it->second;
    }
    

    if (ChunkMesher::uploadMeshData(*mesh, message.meshData)) {
        // Add to active list if its not there
        std::lock_guard<std::mutex> l(lckActiveChunkMeshes);
        if (mesh->activeMeshesIndex == ACTIVE_MESH_INDEX_NONE) {
            mesh->activeMeshesIndex = m_activeChunkMeshes.size();
            mesh->updateVersion = 0;
            m_activeChunkMeshes.push_back(mesh);
        }
    } else {
        // Remove from active list
        std::lock_guard<std::mutex> l(lckActiveChunkMeshes);
        if (mesh->activeMeshesIndex != ACTIVE_MESH_INDEX_NONE) {
            m_activeChunkMeshes[mesh->activeMeshesIndex] = m_activeChunkMeshes.back();
            m_activeChunkMeshes[mesh->activeMeshesIndex]->activeMeshesIndex = mesh->activeMeshesIndex;
            m_activeChunkMeshes.pop_back();
            mesh->activeMeshesIndex = ACTIVE_MESH_INDEX_NONE;
        }
    }

    // TODO(Ben): come on...
    delete message.meshData;
}

void ChunkMeshManager::updateMeshDistances(const f64v3& cameraPosition) {
    static const f64v3 CHUNK_DIMS(CHUNK_WIDTH);
    // TODO(Ben): Spherical instead?
    std::lock_guard<std::mutex> l(lckActiveChunkMeshes);
    for (auto& mesh : m_activeChunkMeshes) { //update distances for all chunk meshes
        //calculate distance
        f64v3 closestPoint = getClosestPointOnAABB(cameraPosition, mesh->position, CHUNK_DIMS);
        // Omit sqrt for faster calculation
        mesh->distance2 = selfDot(closestPoint - cameraPosition);
    }
}

void ChunkMeshManager::onAddSphericalVoxelComponent(Sender s, SphericalVoxelComponent& cmp, vecs::EntityID e) {
    for (ui32 i = 0; i < 6; i++) {
        for (ui32 j = 0; j < cmp.chunkGrids[i].numGenerators; j++) {
            cmp.chunkGrids[i].generators[j].onGenFinish += makeDelegate(*this, &ChunkMeshManager::onGenFinish);
        }
        cmp.chunkGrids[i].onNeighborsAcquire += makeDelegate(*this, &ChunkMeshManager::onNeighborsAcquire);
        cmp.chunkGrids[i].onNeighborsRelease += makeDelegate(*this, &ChunkMeshManager::onNeighborsRelease);
    }
}

void ChunkMeshManager::onRemoveSphericalVoxelComponent(Sender s, SphericalVoxelComponent& cmp, vecs::EntityID e) {
    for (ui32 i = 0; i < 6; i++) {
        for (ui32 j = 0; j < cmp.chunkGrids[i].numGenerators; j++) {
            cmp.chunkGrids[i].generators[j].onGenFinish -= makeDelegate(*this, &ChunkMeshManager::onGenFinish);
        }
        cmp.chunkGrids[i].onNeighborsAcquire -= makeDelegate(*this, &ChunkMeshManager::onNeighborsAcquire);
        cmp.chunkGrids[i].onNeighborsRelease -= makeDelegate(*this, &ChunkMeshManager::onNeighborsRelease);
    }
}

void ChunkMeshManager::onGenFinish(Sender s, ChunkHandle& chunk, ChunkGenLevel gen) {
    // Can be meshed.
    if (gen == GEN_DONE) {
        // Create message
        if (chunk->numBlocks) {
            ChunkMesh* mesh;
            { // TODO(Ben): With gen beyond GEN_DONE this could be redundantly called.
                std::lock_guard<std::mutex> l(m_lckActiveChunks);
                auto& it = m_activeChunks.find(chunk.getID());
                if (it == m_activeChunks.end()) return;
                mesh = it->second;
            }
            ChunkMeshTask* task = trySetMeshDependencies(chunk);
            if (task) {
                mesh->updateVersion = chunk->updateVersion;
                m_threadPool->addTask(task);
                chunk->remeshFlags = 0;
            }
        }
    }
}

void ChunkMeshManager::onNeighborsAcquire(Sender s, ChunkHandle& chunk) {
    ChunkMesh* mesh = createMesh(chunk);
}

void ChunkMeshManager::onNeighborsRelease(Sender s, ChunkHandle& chunk) {
    chunk->updateVersion = 0;
    // Destroy message
    ChunkMeshUpdateMessage msg;
    ChunkMesh* mesh;
    {
        std::lock_guard<std::mutex> l(m_lckActiveChunks);
        auto& it = m_activeChunks.find(chunk.getID());
        if (it == m_activeChunks.end()) {
            return;
        } else {
            mesh = it->second;
            m_activeChunks.erase(it);
        }
    }

    disposeMesh(mesh);
}
