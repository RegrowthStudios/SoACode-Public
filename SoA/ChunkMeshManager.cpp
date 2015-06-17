#include "stdafx.h"
#include "ChunkMeshManager.h"

#include "ChunkMesh.h"
#include "ChunkRenderer.h"
#include "RenderTask.h"
#include "soaUtils.h"

#define MAX_UPDATES_PER_FRAME 100

ChunkMeshManager::ChunkMeshManager(ui32 startMeshes = 128) {
    m_meshStorage.resize(startMeshes);
    m_freeMeshes.resize(startMeshes);
    for (ui32 i = 0; i < startMeshes; i++) {
        m_freeMeshes[i] = i;
    }
}

ChunkMeshManager::~ChunkMeshManager() {
    destroy();
}

void ChunkMeshManager::update(const f64v3& cameraPosition, bool shouldSort) {
    ChunkMeshMessage updateBuffer[MAX_UPDATES_PER_FRAME];
    size_t numUpdates;
    if (numUpdates = m_messages.try_dequeue_bulk(updateBuffer, MAX_UPDATES_PER_FRAME)) {
        for (size_t i = 0; i < numUpdates; i++) {
            updateMesh(updateBuffer[i]);
        }
    }

    // TODO(Ben): This is redundant with the chunk manager! Find a way to share! (Pointer?)
    updateMeshDistances(cameraPosition);
    if (shouldSort) {
        // TODO(Ben): std::sort
    }
}

void ChunkMeshManager::destroy() {

    // Free all chunk meshes
    for (ChunkMesh* cm : m_activeChunkMeshes) {
        delete cm;
    }
    std::vector<ChunkMesh*>().swap(m_activeChunkMeshes);
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

    // Zero buffers
    memset(mesh.vbos, 0, sizeof(mesh.vbos));
    memset(mesh.vaos, 0, sizeof(mesh.vbos));
    mesh.transIndexID = 0;

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

    // Release the mesh object
    m_freeMeshes.push_back(id);
    m_activeChunks.erase(it);
}

void ChunkMeshManager::updateMesh(ChunkMeshMessage& message) {   
    // Get the mesh object
    auto& it = m_activeChunks.find(message.chunkID);
    if (it == m_activeChunks.end()) return; /// The mesh was released, so ignore! // TODO(Ben): MEMORY LEAK!!)(&@!%

    ChunkMesh &cm = m_meshStorage[it->second];

    ChunkMeshData* meshData = message.meshData;

    //store the index data for sorting in the chunk mesh
    cm.transQuadIndices.swap(meshData->transQuadIndices);
    cm.transQuadPositions.swap(meshData->transQuadPositions);

    bool canRender = false;

    switch (meshData->type) {
        case RenderTaskType::DEFAULT:
            if (meshData->vertices.size()) {

                mapBufferData(cm.vboID, meshData->vertices.size() * sizeof(BlockVertex), &(meshData->vertices[0]), GL_STATIC_DRAW);
                canRender = true;

                if (!cm.vaoID) ChunkRenderer::buildVao(cm);
            } else {
                if (cm.vboID != 0) {
                    glDeleteBuffers(1, &(cm.vboID));
                    cm.vboID = 0;
                }
                if (cm.vaoID != 0) {
                    glDeleteVertexArrays(1, &(cm.vaoID));
                    cm.vaoID = 0;
                }
            }

            if (meshData->transVertices.size()) {

                //vertex data
                mapBufferData(cm.transVboID, meshData->transVertices.size() * sizeof(BlockVertex), &(meshData->transVertices[0]), GL_STATIC_DRAW);

                //index data
                mapBufferData(cm.transIndexID, cm.transQuadIndices.size() * sizeof(ui32), &(cm.transQuadIndices[0]), GL_STATIC_DRAW);
                canRender = true;
                cm.needsSort = true; //must sort when changing the mesh

                if (!cm.transVaoID) ChunkRenderer::buildTransparentVao(cm);
            } else {
                if (cm.transVaoID != 0) {
                    glDeleteVertexArrays(1, &(cm.transVaoID));
                    cm.transVaoID = 0;
                }
                if (cm.transVboID != 0) {
                    glDeleteBuffers(1, &(cm.transVboID));
                    cm.transVboID = 0;
                }
                if (cm.transIndexID != 0) {
                    glDeleteBuffers(1, &(cm.transIndexID));
                    cm.transIndexID = 0;
                }
            }

            if (meshData->cutoutVertices.size()) {

                mapBufferData(cm.cutoutVboID, meshData->cutoutVertices.size() * sizeof(BlockVertex), &(meshData->cutoutVertices[0]), GL_STATIC_DRAW);
                canRender = true;
                if (!cm.cutoutVaoID) ChunkRenderer::buildCutoutVao(cm);
            } else {
                if (cm.cutoutVaoID != 0) {
                    glDeleteVertexArrays(1, &(cm.cutoutVaoID));
                    cm.cutoutVaoID = 0;
                }
                if (cm.cutoutVboID != 0) {
                    glDeleteBuffers(1, &(cm.cutoutVboID));
                    cm.cutoutVboID = 0;
                }
            }
            cm.renderData = meshData->chunkMeshRenderData;
        //The missing break is deliberate!
        case RenderTaskType::LIQUID:

            cm.renderData.waterIndexSize = meshData->chunkMeshRenderData.waterIndexSize;
            if (meshData->waterVertices.size()) {
                mapBufferData(cm.waterVboID, meshData->waterVertices.size() * sizeof(LiquidVertex), &(meshData->waterVertices[0]), GL_STREAM_DRAW);
                canRender = true;
                if (!cm.waterVaoID) ChunkRenderer::buildWaterVao(cm);
            } else {
                if (cm.waterVboID != 0) {
                    glDeleteBuffers(1, &(cm.waterVboID));
                    cm.waterVboID = 0;
                }
                if (cm.waterVaoID != 0) {
                    glDeleteVertexArrays(1, &(cm.waterVaoID));
                    cm.waterVaoID = 0;
                }
            }
            break;
    }

    if (canRender) {

    } else {

    }

    // TODO(Ben): come on...
    delete meshData;
}

void ChunkMeshManager::updateMeshDistances(const f64v3& cameraPosition) {
    static const f64v3 CHUNK_DIMS(CHUNK_WIDTH);

    for (auto& cm : m_activeChunkMeshes) { //update distances for all chunk meshes
        //calculate distance
        f64v3 closestPoint = getClosestPointOnAABB(cameraPosition, cm->position, CHUNK_DIMS);
        // Omit sqrt for faster calculation
        cm->distance2 = selfDot(closestPoint - cameraPosition);
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