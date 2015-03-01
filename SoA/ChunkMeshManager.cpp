#include "stdafx.h"
#include "ChunkMeshManager.h"

#include "ChunkMesh.h"
#include "RenderTask.h"
#include "ChunkRenderer.h"

#define MAX_UPDATES_PER_FRAME 100

ChunkMeshManager::ChunkMeshManager() : 
    m_updateBuffer(MAX_UPDATES_PER_FRAME, nullptr) {
    // Empty
}

ChunkMeshManager::~ChunkMeshManager() {
    destroy();
}

void ChunkMeshManager::update(const f64v3& cameraPosition, bool shouldSort) {
    size_t numUpdates;
    if (numUpdates = m_meshQueue.try_dequeue_bulk(m_updateBuffer.begin(), MAX_UPDATES_PER_FRAME)) {
        for (int i = 0; i < numUpdates; i++) {
            updateMesh(m_updateBuffer[i]);
        }
    }

  //  updateMeshDistances(cameraPosition);
    if (shouldSort) {
        // TODO(Ben): std::sort
    }
}

void ChunkMeshManager::deleteMesh(ChunkMesh* mesh, int index /* = -1 */) {
    if (index != -1) {
        m_chunkMeshes[index] = m_chunkMeshes.back();
        m_chunkMeshes.pop_back();
    }
    if (mesh->refCount == 0) {
        delete mesh;
    }
}

void ChunkMeshManager::addMeshForUpdate(ChunkMeshData* meshData) {
    m_meshQueue.enqueue(meshData);
}

void ChunkMeshManager::destroy() {

    // Free all chunk meshes
    for (ChunkMesh* cm : m_chunkMeshes) {
        delete cm;
    }
    std::vector<ChunkMesh*>().swap(m_chunkMeshes);
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

void ChunkMeshManager::updateMesh(ChunkMeshData* meshData) {
    ChunkMesh *cm = meshData->chunkMesh;

    cm->refCount--;

    // Destroy if need be
    if (cm->needsDestroy) {
        if (!cm->inMeshList) {
            deleteMesh(cm);
        }
        delete meshData;
        return;
    }

    //store the index data for sorting in the chunk mesh
    cm->transQuadIndices.swap(meshData->transQuadIndices);
    cm->transQuadPositions.swap(meshData->transQuadPositions);

    switch (meshData->type) {
        case RenderTaskType::DEFAULT:
            if (meshData->vertices.size()) {

                mapBufferData(cm->vboID, meshData->vertices.size() * sizeof(BlockVertex), &(meshData->vertices[0]), GL_STATIC_DRAW);

                ChunkRenderer::bindVao(cm);
            } else {
                if (cm->vboID != 0) {
                    glDeleteBuffers(1, &(cm->vboID));
                    cm->vboID = 0;
                }
                if (cm->vaoID != 0) {
                    glDeleteVertexArrays(1, &(cm->vaoID));
                    cm->vaoID = 0;
                }
            }

            if (meshData->transVertices.size()) {

                //vertex data
                mapBufferData(cm->transVboID, meshData->transVertices.size() * sizeof(BlockVertex), &(meshData->transVertices[0]), GL_STATIC_DRAW);

                //index data
                mapBufferData(cm->transIndexID, cm->transQuadIndices.size() * sizeof(ui32), &(cm->transQuadIndices[0]), GL_STATIC_DRAW);

                cm->needsSort = true; //must sort when changing the mesh

                ChunkRenderer::bindTransparentVao(cm);
            } else {
                if (cm->transVaoID != 0) {
                    glDeleteVertexArrays(1, &(cm->transVaoID));
                    cm->transVaoID = 0;
                }
                if (cm->transVboID != 0) {
                    glDeleteBuffers(1, &(cm->transVboID));
                    cm->transVboID = 0;
                }
                if (cm->transIndexID != 0) {
                    glDeleteBuffers(1, &(cm->transIndexID));
                    cm->transIndexID = 0;
                }
            }

            if (meshData->cutoutVertices.size()) {

                mapBufferData(cm->cutoutVboID, meshData->cutoutVertices.size() * sizeof(BlockVertex), &(meshData->cutoutVertices[0]), GL_STATIC_DRAW);

                ChunkRenderer::bindCutoutVao(cm);
            } else {
                if (cm->cutoutVaoID != 0) {
                    glDeleteVertexArrays(1, &(cm->cutoutVaoID));
                    cm->cutoutVaoID = 0;
                }
                if (cm->cutoutVboID != 0) {
                    glDeleteBuffers(1, &(cm->cutoutVboID));
                    cm->cutoutVboID = 0;
                }
            }
            cm->meshInfo = meshData->chunkMeshRenderData;
            //The missing break is deliberate!
        case RenderTaskType::LIQUID:

            cm->meshInfo.waterIndexSize = meshData->chunkMeshRenderData.waterIndexSize;
            if (meshData->waterVertices.size()) {
                mapBufferData(cm->waterVboID, meshData->waterVertices.size() * sizeof(LiquidVertex), &(meshData->waterVertices[0]), GL_STREAM_DRAW);

                ChunkRenderer::bindWaterVao(cm);
            } else {
                if (cm->waterVboID != 0) {
                    glDeleteBuffers(1, &(cm->waterVboID));
                    cm->waterVboID = 0;
                }
                if (cm->waterVaoID != 0) {
                    glDeleteVertexArrays(1, &(cm->waterVaoID));
                    cm->waterVaoID = 0;
                }
            }
            break;
    }

    //If this mesh isn't in use anymore, delete it
    //if ((cm->vboID == 0 && cm->waterVboID == 0 && cm->transVboID == 0 && cm->cutoutVboID == 0) || cm->needsDestroy) {
    //    if (cm->inMeshList) {
    //        // If its already in the list, mark it for lazy deallocation
    //        cm->needsDestroy = true;
    //    } else {
    //        // Otherwise just delete it
    //        deleteMesh(cm);
    //    }
    //}

    // TODO(Ben): We are adding empty meshes here
    if (!cm->inMeshList) {
        m_chunkMeshes.push_back(cm);
        cm->inMeshList = true;
    }

    delete meshData;
}

void ChunkMeshManager::updateMeshDistances(const f64v3& cameraPosition) {
    ChunkMesh *cm;
    int mx, my, mz;
    double dx, dy, dz;
    double cx, cy, cz;

    mx = (int)cameraPosition.x;
    my = (int)cameraPosition.y;
    mz = (int)cameraPosition.z;

    for (int i = 0; i < m_chunkMeshes.size(); i++) { //update distances for all chunk meshes
        cm = m_chunkMeshes[i];
        const glm::ivec3 &cmPos = cm->position;
        cx = (mx <= cmPos.x) ? cmPos.x : ((mx > cmPos.x + CHUNK_WIDTH) ? (cmPos.x + CHUNK_WIDTH) : mx);
        cy = (my <= cmPos.y) ? cmPos.y : ((my > cmPos.y + CHUNK_WIDTH) ? (cmPos.y + CHUNK_WIDTH) : my);
        cz = (mz <= cmPos.z) ? cmPos.z : ((mz > cmPos.z + CHUNK_WIDTH) ? (cmPos.z + CHUNK_WIDTH) : mz);
        dx = cx - mx;
        dy = cy - my;
        dz = cz - mz;
        cm->distance2 = dx*dx + dy*dy + dz*dz;
    }
}
