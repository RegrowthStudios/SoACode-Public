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
    if ((numUpdates = m_messages.try_dequeue_bulk(updateBuffer, MAX_UPDATES_PER_FRAME))) {
        for (size_t i = 0; i < numUpdates; i++) {
            updateMesh(updateBuffer[i]);
        }
    }

    // Update pending meshes
    {
        std::lock_guard<std::mutex> l(m_lckPendingMesh);
        for (auto it = m_pendingMesh.begin(); it != m_pendingMesh.end();) {
            ChunkMeshTask* task = createMeshTask(it->second);
            if (task) {
                {
                    std::lock_guard<std::mutex> l(m_lckActiveChunks);
                    m_activeChunks[it->first]->updateVersion = it->second->updateVersion;
                }
                m_threadPool->addTask(task);
                it->second.release();
                m_pendingMesh.erase(it++);
            } else {
                ++it;
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

    return mesh;
}

ChunkMeshTask* ChunkMeshManager::createMeshTask(ChunkHandle& chunk) {
    ChunkHandle& left = chunk->neighbor.left;
    ChunkHandle& right = chunk->neighbor.right;
    ChunkHandle& bottom = chunk->neighbor.bottom;
    ChunkHandle& top = chunk->neighbor.top;
    ChunkHandle& back = chunk->neighbor.back;
    ChunkHandle& front = chunk->neighbor.front;

    if (!left.isAquired() || !right.isAquired() || !back.isAquired() ||
        !front.isAquired() || !bottom.isAquired() || !top.isAquired()) {
        std::cout << "NOT ACQUIRED";
    }

    if (left->genLevel != GEN_DONE || right->genLevel != GEN_DONE ||
        back->genLevel != GEN_DONE || front->genLevel != GEN_DONE ||
        bottom->genLevel != GEN_DONE || top->genLevel != GEN_DONE) return nullptr;

    // TODO(Ben): Recycler
    ChunkMeshTask* meshTask = new ChunkMeshTask;
    meshTask->init(chunk, MeshTaskType::DEFAULT, m_blockPack, this);

    // Set dependencies
    meshTask->neighborHandles[NEIGHBOR_HANDLE_LEFT] = left.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_RIGHT] = right.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_FRONT] = front.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_BACK] = back.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_TOP] = top.acquire();
    meshTask->neighborHandles[NEIGHBOR_HANDLE_BOT] = bottom.acquire();
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
        auto it = m_activeChunks.find(message.chunkID);
        if (it == m_activeChunks.end()) {
            delete message.meshData;
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

void ChunkMeshManager::onAddSphericalVoxelComponent(Sender s VORB_UNUSED, SphericalVoxelComponent& cmp, vecs::EntityID e VORB_UNUSED) {
    for (ui32 i = 0; i < 6; i++) {
        for (ui32 j = 0; j < cmp.chunkGrids[i].numGenerators; j++) {
            cmp.chunkGrids[i].generators[j].onGenFinish += makeDelegate(*this, &ChunkMeshManager::onGenFinish);
        }
        cmp.chunkGrids[i].onNeighborsAcquire += makeDelegate(*this, &ChunkMeshManager::onNeighborsAcquire);
        cmp.chunkGrids[i].onNeighborsRelease += makeDelegate(*this, &ChunkMeshManager::onNeighborsRelease);
        Chunk::DataChange += makeDelegate(*this, &ChunkMeshManager::onDataChange);
    }
}

void ChunkMeshManager::onRemoveSphericalVoxelComponent(Sender s VORB_UNUSED, SphericalVoxelComponent& cmp, vecs::EntityID e VORB_UNUSED) {
    for (ui32 i = 0; i < 6; i++) {
        for (ui32 j = 0; j < cmp.chunkGrids[i].numGenerators; j++) {
            cmp.chunkGrids[i].generators[j].onGenFinish -= makeDelegate(*this, &ChunkMeshManager::onGenFinish);
        }
        cmp.chunkGrids[i].onNeighborsAcquire -= makeDelegate(*this, &ChunkMeshManager::onNeighborsAcquire);
        cmp.chunkGrids[i].onNeighborsRelease -= makeDelegate(*this, &ChunkMeshManager::onNeighborsRelease);
        Chunk::DataChange -= makeDelegate(*this, &ChunkMeshManager::onDataChange);
    }
}

void ChunkMeshManager::onGenFinish(Sender s VORB_UNUSED, ChunkHandle& chunk, ChunkGenLevel gen VORB_UNUSED) {
    // Check if can be meshed.
    if (chunk->genLevel == GEN_DONE && chunk->neighbor.left.isAquired() && chunk->numBlocks) {
        std::lock_guard<std::mutex> l(m_lckPendingMesh);
        m_pendingMesh.emplace(chunk.getID(), chunk.acquire());
    }
}

void ChunkMeshManager::onNeighborsAcquire(Sender s VORB_UNUSED, ChunkHandle& chunk) {
    ChunkMesh* mesh = createMesh(chunk);
    // Check if can be meshed.
    if (chunk->genLevel == GEN_DONE && chunk->numBlocks) {
        std::lock_guard<std::mutex> l(m_lckPendingMesh);
        m_pendingMesh.emplace(chunk.getID(), chunk.acquire());
    }
}

void ChunkMeshManager::onNeighborsRelease(Sender s VORB_UNUSED, ChunkHandle& chunk) {
    // Destroy message
    ChunkMesh* mesh;
    {
        std::lock_guard<std::mutex> l(m_lckActiveChunks);
        auto it = m_activeChunks.find(chunk.getID());
        if (it == m_activeChunks.end()) {
            return;
        } else {
            mesh = it->second;
            m_activeChunks.erase(it);
        }
    }
    {
        std::lock_guard<std::mutex> l(m_lckPendingMesh);
        auto it = m_pendingMesh.find(chunk.getID());
        if (it != m_pendingMesh.end()) {
            it->second.release();
            m_pendingMesh.erase(it);
        }
    }

    disposeMesh(mesh);
}

void ChunkMeshManager::onDataChange(Sender s VORB_UNUSED, ChunkHandle& chunk) {
    // Have to have neighbors
    // TODO(Ben): Race condition with neighbor removal here.
    if (chunk->neighbor.left.isAquired()) {
        std::lock_guard<std::mutex> l(m_lckPendingMesh);
        m_pendingMesh.emplace(chunk.getID(), chunk.acquire());
    }
}
