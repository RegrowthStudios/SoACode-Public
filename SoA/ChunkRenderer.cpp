#include "stdafx.h"
#include "ChunkRenderer.h"

#include "Camera.h"
#include "NChunk.h"
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
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pyVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.pyVboOff * 6 * sizeof(GLuint)) / 4));
    }

    //front
    if (chunkMeshInfo.pzVboSize && PlayerPos.z > cm->position.z + chunkMeshInfo.lowestZ){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pzVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.pzVboOff * 6 * sizeof(GLuint)) / 4));
    }

    //back
    if (chunkMeshInfo.nzVboSize && PlayerPos.z < cm->position.z + chunkMeshInfo.highestZ){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nzVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.nzVboOff * 6 * sizeof(GLuint)) / 4));
    }

    //left
    if (chunkMeshInfo.nxVboSize && PlayerPos.x < cm->position.x + chunkMeshInfo.highestX){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nxVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.nxVboOff * 6 * sizeof(GLuint)) / 4));
    }

    //right
    if (chunkMeshInfo.pxVboSize && PlayerPos.x > cm->position.x + chunkMeshInfo.lowestX){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pxVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.pxVboOff * 6 * sizeof(GLuint)) / 4));
    }

    //bottom
    if (chunkMeshInfo.nyVboSize && PlayerPos.y < cm->position.y + chunkMeshInfo.highestY){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nyVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.nyVboOff * 6 * sizeof(GLuint)) / 4));
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

void ChunkRenderer::buildTransparentVao(ChunkMesh& cm)
{
    glGenVertexArrays(1, &(cm.transVaoID));
    glBindVertexArray(cm.transVaoID);

    glBindBuffer(GL_ARRAY_BUFFER, cm.transVboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cm.transIndexID);

    for (int i = 0; i < 8; i++) {
        glEnableVertexAttribArray(i);
    }

    //position + texture type
    glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), 0);
    //UV, animation, blendmode
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (4)));
    //textureAtlas_textureIndex
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (8)));
    //Texture dimensions
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (12)));
    //color
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (16)));
    //overlayColor
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (20)));
    //lightcolor[3], sunlight,
    glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (24)));
    //normal
    glVertexAttribPointer(7, 3, GL_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (28)));

    glBindVertexArray(0);
}

void ChunkRenderer::buildCutoutVao(ChunkMesh& cm)
{
    glGenVertexArrays(1, &(cm.cutoutVaoID));
    glBindVertexArray(cm.cutoutVaoID);

    glBindBuffer(GL_ARRAY_BUFFER, cm.cutoutVboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NChunk::vboIndicesID);

    for (int i = 0; i < 8; i++) {
        glEnableVertexAttribArray(i);
    }

    //position + texture type
    glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), 0);
    //UV, animation, blendmode
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (4)));
    //textureAtlas_textureIndex
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (8)));
    //Texture dimensions
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), ((char *)NULL + (12)));
    //color
    glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (16)));
    //overlayColor
    glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (20)));
    //lightcolor[3], sunlight,
    glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (24)));
    //normal
    glVertexAttribPointer(7, 3, GL_BYTE, GL_TRUE, sizeof(BlockVertex), ((char *)NULL + (28)));


    glBindVertexArray(0);
}

void ChunkRenderer::buildVao(ChunkMesh& cm)
{
    glGenVertexArrays(1, &(cm.vaoID));
    glBindVertexArray(cm.vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, cm.vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NChunk::vboIndicesID);

    for (int i = 0; i < 8; i++) {
        glEnableVertexAttribArray(i);
    }

    //position + texture type
    glVertexAttribPointer(0, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), offsetptr(BlockVertex, position));
    //UV, animation, blendmode
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), offsetptr(BlockVertex, tex));
    //textureAtlas_textureIndex
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), offsetptr(BlockVertex, textureAtlas));
    //Texture dimensions
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(BlockVertex), offsetptr(BlockVertex, textureWidth));
    //color
    glVertexAttribPointer(4, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), offsetptr(BlockVertex, color));
    //overlayColor
    glVertexAttribPointer(5, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), offsetptr(BlockVertex, overlayColor));
    //lightcolor[3], sunlight,
    glVertexAttribPointer(6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(BlockVertex), offsetptr(BlockVertex, lampColor));
    //normal
    glVertexAttribPointer(7, 3, GL_BYTE, GL_TRUE, sizeof(BlockVertex), offsetptr(BlockVertex, normal));

    glBindVertexArray(0);
}

void ChunkRenderer::buildWaterVao(ChunkMesh& cm)
{
    glGenVertexArrays(1, &(cm.waterVaoID));
    glBindVertexArray(cm.waterVaoID);
    glBindBuffer(GL_ARRAY_BUFFER, cm.waterVboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NChunk::vboIndicesID);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, cm.waterVboID);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LiquidVertex), 0);
    //uvs_texUnit_texIndex
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (12)));
    //color
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (16)));
    //light
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (20)));

    glBindVertexArray(0);
}