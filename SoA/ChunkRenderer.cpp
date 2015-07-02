#include "stdafx.h"
#include "ChunkRenderer.h"

#include "Camera.h"
#include "Chunk.h"
#include "ChunkMeshManager.h"
#include "Frustum.h"
#include "GameManager.h"
#include "GameRenderParams.h"
#include "GeometrySorter.h"
#include "SoaOptions.h"
#include "PhysicsBlocks.h"
#include "RenderUtils.h"
#include "soaUtils.h"

volatile f32 ChunkRenderer::fadeDist = 1.0f;
f32m4 ChunkRenderer::worldMatrix = f32m4(1.0f);

void ChunkRenderer::drawOpaque(const ChunkMesh *cm, const vg::GLProgram& program, const f64v3 &PlayerPos, const f32m4 &VP)
{
    if (cm->vboID == 0) {
        //printf("VBO is 0 in drawChunkBlocks\n");
        return;
    }
    setMatrixTranslation(worldMatrix, f64v3(cm->position), PlayerPos);

    f32m4 MVP = VP * worldMatrix;

    glUniformMatrix4fv(program.getUniform("unWVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(program.getUniform("unW"), 1, GL_FALSE, &worldMatrix[0][0]);

    glBindVertexArray(cm->vaoID);

    const ChunkMeshRenderData& chunkMeshInfo = cm->renderData;
    //top
    if (chunkMeshInfo.pyVboSize && PlayerPos.y > cm->position.y + chunkMeshInfo.lowestY) {
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pyVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.pyVboOff * sizeof(GLuint)));
    }

    //front
    if (chunkMeshInfo.pzVboSize && PlayerPos.z > cm->position.z + chunkMeshInfo.lowestZ){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pzVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.pzVboOff * sizeof(GLuint)));
    }

    //back
    if (chunkMeshInfo.nzVboSize && PlayerPos.z < cm->position.z + chunkMeshInfo.highestZ){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nzVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.nzVboOff * sizeof(GLuint)));
    }

    //left
    if (chunkMeshInfo.nxVboSize && PlayerPos.x < cm->position.x + chunkMeshInfo.highestX){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nxVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.nxVboOff * sizeof(GLuint)));
    }

    //right
    if (chunkMeshInfo.pxVboSize && PlayerPos.x > cm->position.x + chunkMeshInfo.lowestX){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pxVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.pxVboOff * sizeof(GLuint)));
    }

    //bottom
    if (chunkMeshInfo.nyVboSize && PlayerPos.y < cm->position.y + chunkMeshInfo.highestY){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nyVboSize, GL_UNSIGNED_INT, (void*)(chunkMeshInfo.nyVboOff * sizeof(GLuint)));
    }

    glBindVertexArray(0);
}

void ChunkRenderer::drawTransparent(const ChunkMesh *cm, const vg::GLProgram& program, const f64v3 &playerPos, const f32m4 &VP) {
    if (cm->transVaoID == 0) return;

    setMatrixTranslation(worldMatrix, f64v3(cm->position), playerPos);

    f32m4 MVP = VP * worldMatrix;

    glUniformMatrix4fv(program.getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(program.getUniform("M"), 1, GL_FALSE, &worldMatrix[0][0]);

    glBindVertexArray(cm->transVaoID);

    glDrawElements(GL_TRIANGLES, cm->renderData.transVboSize, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

}

void ChunkRenderer::drawCutout(const ChunkMesh *cm, const vg::GLProgram& program, const f64v3 &playerPos, const f32m4 &VP) {
    if (cm->cutoutVaoID == 0) return;

    setMatrixTranslation(worldMatrix, f64v3(cm->position), playerPos);

    f32m4 MVP = VP * worldMatrix;

    glUniformMatrix4fv(program.getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(program.getUniform("M"), 1, GL_FALSE, &worldMatrix[0][0]);

    glBindVertexArray(cm->cutoutVaoID);

    glDrawElements(GL_TRIANGLES, cm->renderData.cutoutVboSize, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

}

void ChunkRenderer::drawWater(const ChunkMesh *cm, const vg::GLProgram& program, const f64v3 &PlayerPos, const f32m4 &VP)
{
    //use drawWater bool to avoid checking frustum twice
    if (cm->inFrustum && cm->waterVboID){

        setMatrixTranslation(worldMatrix, f64v3(cm->position), PlayerPos);

        f32m4 MVP = VP * worldMatrix;

        glUniformMatrix4fv(program.getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(program.getUniform("M"), 1, GL_FALSE, &worldMatrix[0][0]);

        glBindVertexArray(cm->waterVaoID);

        glDrawElements(GL_TRIANGLES, cm->renderData.waterIndexSize, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
}
