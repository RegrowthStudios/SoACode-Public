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
#include "Planet.h"

inline bool mapBufferData(GLuint& vboID, GLsizeiptr size, void* src, GLenum usage) {
    // Block Vertices
    if (vboID == 0){
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

MeshManager::MeshManager() { 
    // Empty
}

void MeshManager::updateTerrainMesh(TerrainMeshMessage* tmm) {
    TerrainBuffers *tb = tmm->terrainBuffers;

    vcore::GLProgram* program = GameManager::glProgramManager->getProgram("GroundFromAtmosphere");

    if (tmm->indexSize){
        if (tb->vaoID == 0) glGenVertexArrays(1, &(tb->vaoID));
        glBindVertexArray(tb->vaoID);

        if (tb->vboID == 0) glGenBuffers(1, &(tb->vboID));
        glBindBuffer(GL_ARRAY_BUFFER, tb->vboID); // Bind the buffer (vertex array data)
        glBufferData(GL_ARRAY_BUFFER, tmm->index * sizeof(TerrainVertex), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, tmm->index * sizeof(TerrainVertex), &(tmm->verts[0]));

        if (tb->vboIndexID == 0) glGenBuffers(1, &(tb->vboIndexID));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tb->vboIndexID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, tmm->indexSize * sizeof(GLushort), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, tmm->indexSize * sizeof(GLushort), &(tmm->indices[0]));

        //vertices
        glVertexAttribPointer(program->getAttribute("vertexPosition_modelspace"), 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), 0);
        //UVs
        glVertexAttribPointer(program->getAttribute("vertexUV"), 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (12)));
        //normals
        glVertexAttribPointer(program->getAttribute("vertexNormal_modelspace"), 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (20)));
        //colors
        glVertexAttribPointer(program->getAttribute("vertexColor"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (32)));
        //slope color
        glVertexAttribPointer(program->getAttribute("vertexSlopeColor"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (36)));
        //beach color
        //glVertexAttribPointer(program->getAttribute("vertexBeachColor"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(TerrainVertex), ((char *)NULL + (40)));
        //texureUnit, temperature, rainfall, specular
        glVertexAttribPointer(program->getAttribute("texTempRainSpec"), 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(TerrainVertex), ((char *)NULL + (44)));

        program->enableVertexAttribArrays();
        glBindVertexArray(0); // Disable our Vertex Buffer Object  

        if (tmm->treeIndexSize){
            int treeIndex = (tmm->treeIndexSize * 4) / 6;
            glGenBuffers(1, &(tb->treeVboID));
            glBindBuffer(GL_ARRAY_BUFFER, tb->treeVboID); // Bind the buffer (vertex array data)
            glBufferData(GL_ARRAY_BUFFER, treeIndex * sizeof(TreeVertex), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, treeIndex * sizeof(TreeVertex), tmm->treeVerts);
            delete[] tmm->treeVerts;
        } else{
            if (tb->treeVboID != 0) glDeleteBuffers(1, &(tb->treeVboID));
            tb->treeVboID = 0;
        }
        tb->boundingBox = tmm->boundingBox;
        tb->drawX = tmm->drawX;
        tb->drawY = tmm->drawY;
        tb->drawZ = tmm->drawZ;
        tb->worldX = tmm->worldX;
        tb->worldY = tmm->worldY;
        tb->worldZ = tmm->worldZ;
        tb->cullRadius = tmm->cullRadius;
        tb->indexSize = tmm->indexSize;
        tb->treeIndexSize = tmm->treeIndexSize;
        delete[] tmm->verts;
        delete[] tmm->indices;
        if (tb->vecIndex == -1){
            tb->vecIndex = GameManager::planet->drawList[tmm->face].size();
            GameManager::planet->drawList[tmm->face].push_back(tb);
        }
    } else{
        if (tb->vecIndex != -1){
            GameManager::planet->drawList[tmm->face][tb->vecIndex] = GameManager::planet->drawList[tmm->face].back();
            GameManager::planet->drawList[tmm->face][tb->vecIndex]->vecIndex = tb->vecIndex;
            GameManager::planet->drawList[tmm->face].pop_back();
        }
        if (tb->vaoID != 0) glDeleteVertexArrays(1, &(tb->vaoID));
        if (tb->vboID != 0) glDeleteBuffers(1, &(tb->vboID));
        if (tb->treeVboID != 0) glDeleteBuffers(1, &(tb->treeVboID));
        if (tb->vboIndexID != 0) glDeleteBuffers(1, &(tb->vboIndexID));
        delete tb; //possible race condition
    }
    delete tmm;
}

void MeshManager::updateChunkMesh(ChunkMeshData* cmd) {
    ChunkMesh *cm = cmd->chunkMesh;

    if (cmd->chunkMesh == NULL){
        pError("Chunkmesh == NULL : " + to_string(cmd->debugCode));
        printf(" Chunkmesh == NULL");
    }

    //store the index data for sorting in the chunk mesh
    cm->transQuadIndices.swap(cmd->transQuadIndices);
    cm->transQuadPositions.swap(cmd->transQuadPositions);

    switch (cmd->type) {
        case MeshJobType::DEFAULT:
            if (cmd->vertices.size()) {
                if (cm->vecIndex == -1){
                    cm->vecIndex = _chunkMeshes.size();
                    _chunkMeshes.push_back(cm);
                }

                mapBufferData(cm->vboID, cmd->vertices.size() * sizeof(BlockVertex), &(cmd->vertices[0]), GL_STATIC_DRAW);

                ChunkRenderer::bindVao(cm);
            } else {
                if (cm->vboID != 0){
                    glDeleteBuffers(1, &(cm->vboID));
                    cm->vboID = 0;
                }
                if (cm->vaoID != 0){
                    glDeleteVertexArrays(1, &(cm->vaoID));
                    cm->vaoID = 0;
                }
            }

            if (cmd->transVertices.size()) {
                if (cm->vecIndex == -1){
                    cm->vecIndex = _chunkMeshes.size();
                    _chunkMeshes.push_back(cm);
                }

                //vertex data
                mapBufferData(cm->transVboID, cmd->transVertices.size() * sizeof(BlockVertex), &(cmd->transVertices[0]), GL_STATIC_DRAW);

                //index data
                mapBufferData(cm->transIndexID, cm->transQuadIndices.size() * sizeof(ui32), &(cm->transQuadIndices[0]), GL_STATIC_DRAW);

                cm->needsSort = true; //must sort when changing the mesh

                ChunkRenderer::bindTransparentVao(cm);
            } else {
                if (cm->transVaoID != 0){
                    glDeleteVertexArrays(1, &(cm->transVaoID));
                    cm->transVaoID = 0;
                }
                if (cm->transVboID == 0) {
                    glDeleteBuffers(1, &(cm->transVboID));
                    cm->transVboID = 0;
                }
                if (cm->transIndexID == 0) {
                    glDeleteBuffers(1, &(cm->transIndexID));
                    cm->transIndexID = 0;
                }
            }

            if (cmd->cutoutVertices.size()) {
                if (cm->vecIndex == -1){
                    cm->vecIndex = _chunkMeshes.size();
                    _chunkMeshes.push_back(cm);
                }

                mapBufferData(cm->cutoutVboID, cmd->cutoutVertices.size() * sizeof(BlockVertex), &(cmd->cutoutVertices[0]), GL_STATIC_DRAW);

                ChunkRenderer::bindCutoutVao(cm);
            } else {
                if (cm->cutoutVaoID != 0){
                    glDeleteVertexArrays(1, &(cm->cutoutVaoID));
                    cm->cutoutVaoID = 0;
                }
                if (cm->cutoutVboID == 0) {
                    glDeleteBuffers(1, &(cm->cutoutVboID));
                    cm->cutoutVboID = 0;
                }
            }
            cm->meshInfo = cmd->meshInfo;
            //The missing break is deliberate!
        case MeshJobType::LIQUID:

            cm->meshInfo.waterIndexSize = cmd->meshInfo.waterIndexSize;
            if (cmd->waterVertices.size()) {
                if (cm->vecIndex == -1){
                    cm->vecIndex = _chunkMeshes.size();
                    _chunkMeshes.push_back(cm);
                }

                mapBufferData(cm->waterVboID, cmd->waterVertices.size() * sizeof(LiquidVertex), &(cmd->waterVertices[0]), GL_STREAM_DRAW);

                ChunkRenderer::bindWaterVao(cm);
            } else {
                if (cm->waterVboID != 0){
                    glDeleteBuffers(1, &(cm->waterVboID));
                    cm->waterVboID = 0;
                }
                if (cm->waterVaoID != 0){
                    glDeleteVertexArrays(1, &(cm->waterVaoID));
                    cm->waterVaoID = 0;
                }
            }
            break;
    }

    //If this mesh isnt in use anymore, delete it
    if (cm->vboID == 0 && cm->waterVboID == 0 && cm->transVboID == 0 && cm->cutoutVboID == 0){
        if (cm->vecIndex != -1){
            _chunkMeshes[cm->vecIndex] = _chunkMeshes.back();
            _chunkMeshes[cm->vecIndex]->vecIndex = cm->vecIndex;
            _chunkMeshes.pop_back();
        }
        delete cm;
    }


    delete cmd;
}

void MeshManager::updateParticleMesh(ParticleMeshMessage* pmm) {
    ParticleMesh *pm = pmm->mesh;
    int n = pmm->verts.size();

    if (n != 0){
        if (pm->uvBufferID == 0){
            glGenBuffers(1, &(pm->uvBufferID));
            glGenBuffers(1, &(pm->billboardVertexBufferID));
            glBindBuffer(GL_ARRAY_BUFFER, pm->uvBufferID);
            glBufferData(GL_ARRAY_BUFFER, sizeof(particleUVs), particleUVs, GL_STATIC_DRAW);
        }

        pm->usedParticles.swap(pmm->usedParticles);
        pm->size = pmm->size;
        pm->X = pmm->X;
        pm->Y = pmm->Y;
        pm->Z = pmm->Z;

        glBindBuffer(GL_ARRAY_BUFFER, pm->billboardVertexBufferID); // Bind the buffer (vertex array data)
        glBufferData(GL_ARRAY_BUFFER, n * sizeof(BillboardVertex), NULL, GL_STREAM_DRAW);
        void *v = glMapBufferRange(GL_ARRAY_BUFFER, 0, n * sizeof(BillboardVertex), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        memcpy(v, &(pmm->verts[0]), n * sizeof(BillboardVertex));
        glUnmapBuffer(GL_ARRAY_BUFFER);

        if (pm->vecIndex == -1){
            pm->vecIndex = _particleMeshes.size();
            _particleMeshes.push_back(pm);
        }
    } else{ //clear buffers
        if (pm->uvBufferID != 0){
            glDeleteBuffers(1, &pm->billboardVertexBufferID);
            glDeleteBuffers(1, &pm->uvBufferID);
            pm->billboardVertexBufferID = 0;
            pm->uvBufferID = 0;
        }
    }
    delete pmm;
}

void MeshManager::updatePhysicsBlockMesh(PhysicsBlockMeshMessage* pbmm) {
    PhysicsBlockMesh *pbm = pbmm->mesh;
    if (pbmm->verts.size() != 0){
        glGenBuffers(1, &pbm->vboID); // Create the buffer ID
        glBindBuffer(GL_ARRAY_BUFFER, pbm->vboID); // Bind the buffer (vertex array data)
        glBufferData(GL_ARRAY_BUFFER, pbmm->verts.size() * sizeof(PhysicsBlockVertex), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, pbmm->verts.size() * sizeof(PhysicsBlockVertex), &(pbmm->verts[0]));

        if (pbm->vecIndex == -1){
            pbm->vecIndex = _physicsBlockMeshes.size();
            _physicsBlockMeshes.push_back(pbm);
        }
    } else if (pbmm->posLight.size() != 0){

        pbm->bX = pbmm->bX;
        pbm->bY = pbmm->bY;
        pbm->bZ = pbmm->bZ;
        pbm->numBlocks = pbmm->numBlocks;

        if (pbm->positionLightBufferID == 0){
            glGenBuffers(1, &pbm->positionLightBufferID);
        }
        glBindBuffer(GL_ARRAY_BUFFER, pbm->positionLightBufferID);
        glBufferData(GL_ARRAY_BUFFER, pbmm->posLight.size() * sizeof(PhysicsBlockPosLight), NULL, GL_STREAM_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, pbmm->posLight.size() * sizeof(PhysicsBlockPosLight), &(pbmm->posLight[0]));

        if (pbm->vecIndex == -1){
            pbm->vecIndex = _physicsBlockMeshes.size();
            _physicsBlockMeshes.push_back(pbm);
        }
    } else{ //delete
        if (pbm->vecIndex != -1){
            _physicsBlockMeshes[pbm->vecIndex] = _physicsBlockMeshes.back();
            _physicsBlockMeshes[pbm->vecIndex]->vecIndex = pbm->vecIndex;
            _physicsBlockMeshes.pop_back();
        }
        if (pbm->vboID != 0){
            glDeleteBuffers(1, &pbm->vboID);
        }
        if (pbm->positionLightBufferID != 0){
            glDeleteBuffers(1, &pbm->positionLightBufferID);
        }
        delete pbm;
    }
    delete pbmm;
}

void MeshManager::sortMeshes(const f64v3& cameraPosition) {
    updateMeshDistances(cameraPosition);
    recursiveSortMeshList(_chunkMeshes, 0, _chunkMeshes.size());
}

void MeshManager::destroy() {

    // Free all chunk meshes
    for (ChunkMesh* cm : _chunkMeshes) {
        if (cm->vboID != 0){
            glDeleteBuffers(1, &(cm->vboID));
        }
        if (cm->vaoID != 0){
            glDeleteVertexArrays(1, &(cm->vaoID));
        }
        if (cm->transVaoID != 0){
            glDeleteVertexArrays(1, &(cm->transVaoID));
        }
        if (cm->transVboID == 0) {
            glDeleteBuffers(1, &(cm->transVboID));
        }
        if (cm->transIndexID == 0) {
            glDeleteBuffers(1, &(cm->transIndexID));
        }
        if (cm->cutoutVaoID != 0){
            glDeleteVertexArrays(1, &(cm->cutoutVaoID));
        }
        if (cm->cutoutVboID == 0) {
            glDeleteBuffers(1, &(cm->cutoutVboID));
        }
        if (cm->waterVaoID != 0){
            glDeleteBuffers(1, &(cm->waterVaoID));
        }
        if (cm->waterVboID != 0){
            glDeleteBuffers(1, &(cm->waterVboID));
        }
    }
    std::vector<ChunkMesh*>().swap(_chunkMeshes);

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

void MeshManager::updateMeshDistances(const f64v3& cameraPosition) {
    ChunkMesh *cm;
    int mx, my, mz;
    double dx, dy, dz;
    double cx, cy, cz;

    mx = (int)cameraPosition.x;
    my = (int)cameraPosition.y;
    mz = (int)cameraPosition.z;
    //GLuint sticks = SDL_GetTicks();

    static GLuint saveTicks = SDL_GetTicks();

    for (int i = 0; i < _chunkMeshes.size(); i++){ //update distances for all chunk meshes
        cm = _chunkMeshes[i];
        const glm::ivec3 &cmPos = cm->position;
        cx = (mx <= cmPos.x) ? cmPos.x : ((mx > cmPos.x + CHUNK_WIDTH) ? (cmPos.x + CHUNK_WIDTH) : mx);
        cy = (my <= cmPos.y) ? cmPos.y : ((my > cmPos.y + CHUNK_WIDTH) ? (cmPos.y + CHUNK_WIDTH) : my);
        cz = (mz <= cmPos.z) ? cmPos.z : ((mz > cmPos.z + CHUNK_WIDTH) ? (cmPos.z + CHUNK_WIDTH) : mz);
        dx = cx - mx;
        dy = cy - my;
        dz = cz - mz;
        cm->distance = sqrt(dx*dx + dy*dy + dz*dz);
    }
}

void MeshManager::recursiveSortMeshList(std::vector <ChunkMesh*> &v, int start, int size)
{
    if (size < 2) return;
    int i, j;
    ChunkMesh *pivot, *mid, *last, *tmp;

    pivot = v[start];

    //end recursion when small enough
    if (size == 2){
        if ((pivot->distance) < (v[start + 1]->distance)){
            v[start] = v[start + 1];
            v[start + 1] = pivot;

            v[start]->vecIndex = start;
            v[start + 1]->vecIndex = start + 1;

        }
        return;
    }

    mid = v[start + size / 2];
    last = v[start + size - 1];

    //variables to minimize dereferences
    int md, ld, pd;
    pd = pivot->distance;
    md = mid->distance;
    ld = last->distance;

    //calculate pivot
    if ((pd > md && md > ld) || (pd < md && md < ld)){
        v[start] = mid;

        v[start + size / 2] = pivot;


        mid->vecIndex = start;
        pivot->vecIndex = start + size / 2;


        pivot = mid;
        pd = md;
    } else if ((pd > ld && ld > md) || (pd < ld && ld < md)){
        v[start] = last;

        v[start + size - 1] = pivot;


        last->vecIndex = start;
        pivot->vecIndex = start + size - 1;


        pivot = last;
        pd = ld;
    }

    i = start + 1;
    j = start + size - 1;

    //increment and decrement pointers until they are past each other
    while (i <= j){
        while (i < start + size - 1 && (v[i]->distance) > pd) i++;
        while (j > start + 1 && (v[j]->distance) < pd) j--;

        if (i <= j){
            tmp = v[i];
            v[i] = v[j];
            v[j] = tmp;

            v[i]->vecIndex = i;
            v[j]->vecIndex = j;

            i++;
            j--;
        }
    }

    //swap pivot with rightmost element in left set
    v[start] = v[j];
    v[j] = pivot;

    v[start]->vecIndex = start;
    v[j]->vecIndex = j;

    //sort the two subsets excluding the pivot point
    recursiveSortMeshList(v, start, j - start);
    recursiveSortMeshList(v, j + 1, start + size - j - 1);
}