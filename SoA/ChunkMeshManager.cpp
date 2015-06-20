#include "stdafx.h"
#include "ChunkMeshManager.h"

#include "ChunkMesh.h"
#include "ChunkRenderer.h"
#include "RenderTask.h"
#include "soaUtils.h"

#define MAX_UPDATES_PER_FRAME 100

ChunkMeshManager::ChunkMeshManager(ui32 startMeshes /*= 128*/) {
    m_meshStorage.resize(startMeshes);
    m_freeMeshes.resize(startMeshes);
    for (ui32 i = 0; i < startMeshes; i++) {
        m_freeMeshes[i] = i;
    }
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
        // TODO(Ben): std::sort
    }
}

void ChunkMeshManager::destroy() {
    std::vector <ChunkMesh*>().swap(m_activeChunkMeshes);
    moodycamel::ConcurrentQueue<ChunkMeshMessage>().swap(m_messages);
    std::vector <MeshID>().swap(m_freeMeshes);
    std::vector <ChunkMesh>().swap(m_meshStorage);
    std::unordered_map<ChunkID, MeshID>().swap(m_activeChunks);
}

inline bool mapBufferData(GLuint& vboID, GLsizeiptr size, void* src, GLenum usage) {
    // Block Vertices
    if (vboID == 0) {
        glGenBuffers(1, &(vboID)); // Create the buffer ID
    }
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, usage);

    void *v = glMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

    if (v == NULL) return false;

    memcpy(v, src, size);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
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
    // Get a free mesh
    updateMeshStorage();
    MeshID id = m_freeMeshes.back();
    m_freeMeshes.pop_back();
    ChunkMesh& mesh = m_meshStorage[id];

    // Set the position
    mesh.position = static_cast<VoxelPosition3D*>(message.data)->pos;

    // Zero buffers
    memset(mesh.vbos, 0, sizeof(mesh.vbos));
    memset(mesh.vaos, 0, sizeof(mesh.vbos));
    mesh.transIndexID = 0;
    mesh.activeMeshesIndex = ACTIVE_MESH_INDEX_NONE;

    // Register chunk as active and give it a mesh
    m_activeChunks[message.chunkID] = id;
}

void ChunkMeshManager::destroyMesh(ChunkMeshMessage& message) {
    // Get the mesh object
    auto& it = m_activeChunks.find(message.chunkID);
    MeshID& id = it->second;
    ChunkMesh& mesh = m_meshStorage[id];

    // De-allocate buffer objects
    glDeleteBuffers(4, mesh.vbos);
    glDeleteVertexArrays(4, mesh.vaos);
    if (mesh.transIndexID) glDeleteBuffers(1, &mesh.transIndexID);
    mesh.vboID = 0;
    mesh.vaoID = 0;

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
    // Get the mesh object
    auto& it = m_activeChunks.find(message.chunkID);
    if (it == m_activeChunks.end()) return; /// The mesh was already released, so ignore! // TODO(Ben): MEMORY LEAK!!)(&@!%

    // TODO(Ben): According to moodycamel there is a tiny chance of messages being out of order?
    ChunkMesh &mesh = m_meshStorage[it->second];

    ChunkMeshData* meshData = static_cast<ChunkMeshData*>(message.data);

    //store the index data for sorting in the chunk mesh
    mesh.transQuadIndices.swap(meshData->transQuadIndices);
    mesh.transQuadPositions.swap(meshData->transQuadPositions);

    bool canRender = false;

    switch (meshData->type) {
        case RenderTaskType::DEFAULT:
            if (meshData->vertices.size()) {

                mapBufferData(mesh.vboID, meshData->vertices.size() * sizeof(BlockVertex), &(meshData->vertices[0]), GL_STATIC_DRAW);
                canRender = true;

                if (!mesh.vaoID) ChunkRenderer::buildVao(mesh);
            } else {
                if (mesh.vboID != 0) {
                    glDeleteBuffers(1, &(mesh.vboID));
                    mesh.vboID = 0;
                }
                if (mesh.vaoID != 0) {
                    glDeleteVertexArrays(1, &(mesh.vaoID));
                    mesh.vaoID = 0;
                }
            }

            if (meshData->transVertices.size()) {

                //vertex data
                mapBufferData(mesh.transVboID, meshData->transVertices.size() * sizeof(BlockVertex), &(meshData->transVertices[0]), GL_STATIC_DRAW);

                //index data
                mapBufferData(mesh.transIndexID, mesh.transQuadIndices.size() * sizeof(ui32), &(mesh.transQuadIndices[0]), GL_STATIC_DRAW);
                canRender = true;
                mesh.needsSort = true; //must sort when changing the mesh

                if (!mesh.transVaoID) ChunkRenderer::buildTransparentVao(mesh);
            } else {
                if (mesh.transVaoID != 0) {
                    glDeleteVertexArrays(1, &(mesh.transVaoID));
                    mesh.transVaoID = 0;
                }
                if (mesh.transVboID != 0) {
                    glDeleteBuffers(1, &(mesh.transVboID));
                    mesh.transVboID = 0;
                }
                if (mesh.transIndexID != 0) {
                    glDeleteBuffers(1, &(mesh.transIndexID));
                    mesh.transIndexID = 0;
                }
            }

            if (meshData->cutoutVertices.size()) {

                mapBufferData(mesh.cutoutVboID, meshData->cutoutVertices.size() * sizeof(BlockVertex), &(meshData->cutoutVertices[0]), GL_STATIC_DRAW);
                canRender = true;
                if (!mesh.cutoutVaoID) ChunkRenderer::buildCutoutVao(mesh);
            } else {
                if (mesh.cutoutVaoID != 0) {
                    glDeleteVertexArrays(1, &(mesh.cutoutVaoID));
                    mesh.cutoutVaoID = 0;
                }
                if (mesh.cutoutVboID != 0) {
                    glDeleteBuffers(1, &(mesh.cutoutVboID));
                    mesh.cutoutVboID = 0;
                }
            }
            mesh.renderData = meshData->chunkMeshRenderData;
        //The missing break is deliberate!
        case RenderTaskType::LIQUID:

            mesh.renderData.waterIndexSize = meshData->chunkMeshRenderData.waterIndexSize;
            if (meshData->waterVertices.size()) {
                mapBufferData(mesh.waterVboID, meshData->waterVertices.size() * sizeof(LiquidVertex), &(meshData->waterVertices[0]), GL_STREAM_DRAW);
                canRender = true;
                if (!mesh.waterVaoID) ChunkRenderer::buildWaterVao(mesh);
            } else {
                if (mesh.waterVboID != 0) {
                    glDeleteBuffers(1, &(mesh.waterVboID));
                    mesh.waterVboID = 0;
                }
                if (mesh.waterVaoID != 0) {
                    glDeleteVertexArrays(1, &(mesh.waterVaoID));
                    mesh.waterVaoID = 0;
                }
            }
            break;
    }

    // Manage the active list
    if (canRender) {
        if (mesh.activeMeshesIndex == ACTIVE_MESH_INDEX_NONE) {
            mesh.activeMeshesIndex = m_activeChunkMeshes.size();
            m_activeChunkMeshes.push_back(&mesh);
        }
    } else {
        if (mesh.activeMeshesIndex != ACTIVE_MESH_INDEX_NONE) {
            m_activeChunkMeshes[mesh.activeMeshesIndex] = m_activeChunkMeshes.back();
            m_activeChunkMeshes[mesh.activeMeshesIndex]->activeMeshesIndex = mesh.activeMeshesIndex;
            m_activeChunkMeshes.pop_back();
            mesh.activeMeshesIndex = ACTIVE_MESH_INDEX_NONE;
        }
    }

    // TODO(Ben): come on...
    delete meshData;
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
        std::vector<ui32> tmp(m_activeChunkMeshes.size());
        for (size_t i = 0; i < tmp.size(); i++) {
            tmp[i] = m_activeChunkMeshes[i]->activeMeshesIndex;
        }

        // Resize the storage
        ui32 i = m_meshStorage.size();
        m_meshStorage.resize((ui32)(m_meshStorage.size() * 1.5f));
        for (; i < m_meshStorage.size(); i++) {
            m_freeMeshes.push_back(i);
        }

        // Set the pointers again, since they were invalidated
        for (size_t i = 0; i < tmp.size(); i++) {
            m_activeChunkMeshes[i] = &m_meshStorage[i];
        }
    }
}