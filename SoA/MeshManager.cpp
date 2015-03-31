#include "stdafx.h"
#include "MeshManager.h"

#include "TerrainPatch.h"
#include "ChunkMesh.h"
#include "ParticleMesh.h"
#include "PhysicsBlocks.h"
#include "RenderTask.h"
#include "Errors.h"
#include "ChunkRenderer.h"
#include "GameManager.h"


MeshManager::MeshManager() {
    // Empty
}

void MeshManager::updateParticleMesh(ParticleMeshMessage* pmm) {
    //TODO(Ben): Re-implement
    //ParticleMesh *pm = pmm->mesh;
    //int n = pmm->verts.size();

    //if (n != 0){
    //    if (pm->uvBufferID == 0){
    //        glGenBuffers(1, &(pm->uvBufferID));
    //        glGenBuffers(1, &(pm->billboardVertexBufferID));
    //        glBindBuffer(GL_ARRAY_BUFFER, pm->uvBufferID);
    //        glBufferData(GL_ARRAY_BUFFER, sizeof(particleUVs), particleUVs, GL_STATIC_DRAW);
    //    }

    //    pm->usedParticles.swap(pmm->usedParticles);
    //    pm->size = pmm->size;
    //    pm->X = pmm->X;
    //    pm->Y = pmm->Y;
    //    pm->Z = pmm->Z;

    //    glBindBuffer(GL_ARRAY_BUFFER, pm->billboardVertexBufferID); // Bind the buffer (vertex array data)
    //    glBufferData(GL_ARRAY_BUFFER, n * sizeof(BillboardVertex), NULL, GL_STREAM_DRAW);
    //    void *v = glMapBufferRange(GL_ARRAY_BUFFER, 0, n * sizeof(BillboardVertex), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
    //    memcpy(v, &(pmm->verts[0]), n * sizeof(BillboardVertex));
    //    glUnmapBuffer(GL_ARRAY_BUFFER);

    //    if (pm->vecIndex == UNINITIALIZED_INDEX){
    //        pm->vecIndex = _particleMeshes.size();
    //        _particleMeshes.push_back(pm);
    //    }
    //} else{ //clear buffers
    //    if (pm->uvBufferID != 0){
    //        glDeleteBuffers(1, &pm->billboardVertexBufferID);
    //        glDeleteBuffers(1, &pm->uvBufferID);
    //        pm->billboardVertexBufferID = 0;
    //        pm->uvBufferID = 0;
    //    }
    //}
    //delete pmm;
}

void MeshManager::updatePhysicsBlockMesh(PhysicsBlockMeshMessage* pbmm) {
    // TODO(Ben): Re-implement
    //PhysicsBlockMesh *pbm = pbmm->mesh;
    //if (pbmm->verts.size() != 0){
    //    glGenBuffers(1, &pbm->vboID); // Create the buffer ID
    //    glBindBuffer(GL_ARRAY_BUFFER, pbm->vboID); // Bind the buffer (vertex array data)
    //    glBufferData(GL_ARRAY_BUFFER, pbmm->verts.size() * sizeof(PhysicsBlockVertex), NULL, GL_STATIC_DRAW);
    //    glBufferSubData(GL_ARRAY_BUFFER, 0, pbmm->verts.size() * sizeof(PhysicsBlockVertex), &(pbmm->verts[0]));

    //    if (pbm->vecIndex == UNINITIALIZED_INDEX){
    //        pbm->vecIndex = _physicsBlockMeshes.size();
    //        _physicsBlockMeshes.push_back(pbm);
    //    }
    //} else if (pbmm->posLight.size() != 0){
    //    
    //    if (pbm->positionLightBufferID == 0) {
    //        glGenBuffers(1, &pbm->positionLightBufferID);
    //    }

    //    if (pbm->vaoID == 0) {
    //        pbm->createVao(m_glProgramManager->getProgram("PhysicsBlock"));
    //    }

    //    pbm->bX = pbmm->bX;
    //    pbm->bY = pbmm->bY;
    //    pbm->bZ = pbmm->bZ;
    //    pbm->numBlocks = pbmm->numBlocks;

    //    glBindBuffer(GL_ARRAY_BUFFER, pbm->positionLightBufferID);
    //    glBufferData(GL_ARRAY_BUFFER, pbmm->posLight.size() * sizeof(PhysicsBlockPosLight), NULL, GL_STREAM_DRAW);
    //    glBufferSubData(GL_ARRAY_BUFFER, 0, pbmm->posLight.size() * sizeof(PhysicsBlockPosLight), &(pbmm->posLight[0]));

    //    if (pbm->vecIndex == UNINITIALIZED_INDEX){
    //        pbm->vecIndex = _physicsBlockMeshes.size();
    //        _physicsBlockMeshes.push_back(pbm);
    //    }
    //} else{ //delete
    //    if (pbm->vecIndex != UNINITIALIZED_INDEX){
    //        _physicsBlockMeshes[pbm->vecIndex] = _physicsBlockMeshes.back();
    //        _physicsBlockMeshes[pbm->vecIndex]->vecIndex = pbm->vecIndex;
    //        _physicsBlockMeshes.pop_back();
    //    }
    //    if (pbm->vaoID != 0) {
    //        glDeleteBuffers(1, &pbm->vaoID);
    //    }
    //    if (pbm->vboID != 0){
    //        glDeleteBuffers(1, &pbm->vboID);
    //    }
    //    if (pbm->positionLightBufferID != 0){
    //        glDeleteBuffers(1, &pbm->positionLightBufferID);
    //    }
    //    delete pbm;
    //}
    //delete pbmm;
}

void MeshManager::destroy() {
    // Free all particle meshes
    for (ParticleMesh* pm : _particleMeshes) {
        if (pm->billboardVertexBufferID != 0) {
            glDeleteBuffers(1, &pm->billboardVertexBufferID);
        }
        if (pm->uvBufferID != 0) {
            glDeleteBuffers(1, &pm->uvBufferID);
        }
    }
    std::vector<ParticleMesh*>().swap(_particleMeshes);

    // Free all physics block meshes
    for (PhysicsBlockMesh* pmm : _physicsBlockMeshes) {
        if (pmm->vboID != 0) {
            glDeleteBuffers(1, &pmm->vboID);
        }
    }
    std::vector<PhysicsBlockMesh*>().swap(_physicsBlockMeshes);

}
