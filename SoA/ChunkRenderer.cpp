#include "stdafx.h"

#include "ChunkRenderer.h"

#include "Chunk.h"
#include "global.h"
#include "shader.h"

void ChunkRenderer::draw(const ChunkMesh *CMI, const glm::dvec3 &PlayerPos, const glm::mat4 &VP)
{
    if (CMI->vboID == 0) return;

    GlobalModelMatrix[3][0] = ((float)((double)CMI->position.x - PlayerPos.x));
    GlobalModelMatrix[3][1] = ((float)((double)CMI->position.y - PlayerPos.y));
    GlobalModelMatrix[3][2] = ((float)((double)CMI->position.z - PlayerPos.z));

    glm::mat4 MVP = VP * GlobalModelMatrix;

    glUniformMatrix4fv(blockShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(blockShader.mID, 1, GL_FALSE, &GlobalModelMatrix[0][0]);

    glBindVertexArray(CMI->vaoID);

    const ChunkMeshRenderData& chunkMeshInfo = CMI->meshInfo;

    //top
    if (chunkMeshInfo.pyVboSize && PlayerPos.y > CMI->position.y + chunkMeshInfo.lowestY){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pyVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.pyVboOff * 6 * sizeof(GLuint)) / 4));
    }

    //front
    if (chunkMeshInfo.pzVboSize && PlayerPos.z > CMI->position.z + chunkMeshInfo.lowestZ){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pzVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.pzVboOff * 6 * sizeof(GLuint)) / 4));
    }

    //back
    if (chunkMeshInfo.nzVboSize && PlayerPos.z < CMI->position.z + chunkMeshInfo.highestZ){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nzVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.nzVboOff * 6 * sizeof(GLuint)) / 4));
    }

    //left
    if (chunkMeshInfo.nxVboSize && PlayerPos.x < CMI->position.x + chunkMeshInfo.highestX){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nxVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.nxVboOff * 6 * sizeof(GLuint)) / 4));
    }

    //right
    if (chunkMeshInfo.pxVboSize && PlayerPos.x > CMI->position.x + chunkMeshInfo.lowestX){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.pxVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.pxVboOff * 6 * sizeof(GLuint)) / 4));
    }

    //bottom
    if (chunkMeshInfo.nyVboSize && PlayerPos.y < CMI->position.y + chunkMeshInfo.highestY){
        glDrawElements(GL_TRIANGLES, chunkMeshInfo.nyVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.nyVboOff * 6 * sizeof(GLuint)) / 4));
    }
    

    glBindVertexArray(0);
}

void ChunkRenderer::drawTransparentBlocks(const ChunkMesh *CMI, const glm::dvec3 &playerPos, const glm::mat4 &VP) {
    if (CMI->transVboID == 0) return;

    GlobalModelMatrix[3][0] = ((float)((double)CMI->position.x - playerPos.x));
    GlobalModelMatrix[3][1] = ((float)((double)CMI->position.y - playerPos.y));
    GlobalModelMatrix[3][2] = ((float)((double)CMI->position.z - playerPos.z));

    glm::mat4 MVP = VP * GlobalModelMatrix;

    glUniformMatrix4fv(cutoutShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(cutoutShader.mID, 1, GL_FALSE, &GlobalModelMatrix[0][0]);

    glBindVertexArray(CMI->transVaoID);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CMI->transIndexID);

    glDrawElements(GL_TRIANGLES, CMI->meshInfo.transVboSize, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

}

void ChunkRenderer::drawCutoutBlocks(const ChunkMesh *CMI, const glm::dvec3 &playerPos, const glm::mat4 &VP) {
    if (CMI->cutoutVaoID == 0) return;

    GlobalModelMatrix[3][0] = ((float)((double)CMI->position.x - playerPos.x));
    GlobalModelMatrix[3][1] = ((float)((double)CMI->position.y - playerPos.y));
    GlobalModelMatrix[3][2] = ((float)((double)CMI->position.z - playerPos.z));

    glm::mat4 MVP = VP * GlobalModelMatrix;

    glUniformMatrix4fv(cutoutShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(cutoutShader.mID, 1, GL_FALSE, &GlobalModelMatrix[0][0]);

    glBindVertexArray(CMI->cutoutVaoID);

    glDrawElements(GL_TRIANGLES, CMI->meshInfo.cutoutVboSize, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

}

void ChunkRenderer::drawSonar(const ChunkMesh *CMI, const glm::dvec3 &PlayerPos, const glm::mat4 &VP)
{
    if (CMI->inFrustum){
        if (CMI->vboID == 0) return;

        GlobalModelMatrix[3][0] = ((float)((double)CMI->position.x - PlayerPos.x));
        GlobalModelMatrix[3][1] = ((float)((double)CMI->position.y - PlayerPos.y));
        GlobalModelMatrix[3][2] = ((float)((double)CMI->position.z - PlayerPos.z));

        glm::mat4 MVP = VP * GlobalModelMatrix;

        glUniformMatrix4fv(sonarShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(sonarShader.mID, 1, GL_FALSE, &GlobalModelMatrix[0][0]);

        glBindVertexArray(CMI->vaoID);

        const ChunkMeshRenderData& chunkMeshInfo = CMI->meshInfo;

        if (chunkMeshInfo.pyVboSize && PlayerPos.y > CMI->position.y + chunkMeshInfo.lowestY){
            glDrawElements(GL_TRIANGLES, chunkMeshInfo.pyVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.pyVboOff * 6 * sizeof(GLuint)) / 4));
        }

        //front
        if (chunkMeshInfo.pzVboSize && PlayerPos.z > CMI->position.z + chunkMeshInfo.lowestZ){
            glDrawElements(GL_TRIANGLES, chunkMeshInfo.pzVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.pzVboOff * 6 * sizeof(GLuint)) / 4));
        }

        //back
        if (chunkMeshInfo.nzVboSize && PlayerPos.z < CMI->position.z + chunkMeshInfo.highestZ){
            glDrawElements(GL_TRIANGLES, chunkMeshInfo.nzVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.nzVboOff * 6 * sizeof(GLuint)) / 4));
        }

        //left
        if (chunkMeshInfo.nxVboSize && PlayerPos.x < CMI->position.x + chunkMeshInfo.highestX){
            glDrawElements(GL_TRIANGLES, chunkMeshInfo.nxVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.nxVboOff * 6 * sizeof(GLuint)) / 4));
        }

        //right
        if (chunkMeshInfo.pxVboSize && PlayerPos.x > CMI->position.x + chunkMeshInfo.lowestX){
            glDrawElements(GL_TRIANGLES, chunkMeshInfo.pxVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.pxVboOff * 6 * sizeof(GLuint)) / 4));
        }

        //bottom
        if (chunkMeshInfo.nyVboSize && PlayerPos.y < CMI->position.y + chunkMeshInfo.highestY){
            glDrawElements(GL_TRIANGLES, chunkMeshInfo.nyVboSize, GL_UNSIGNED_INT, ((char *)NULL + (chunkMeshInfo.nyVboOff * 6 * sizeof(GLuint)) / 4));
        }

        glBindVertexArray(0);
    }
}

void ChunkRenderer::drawWater(const ChunkMesh *CMI, const glm::dvec3 &PlayerPos, const glm::mat4 &VP)
{
    //use drawWater bool to avoid checking frustum twice
    if (CMI->inFrustum && CMI->waterVboID){

        GlobalModelMatrix[3][0] = (float)((double)CMI->position.x - PlayerPos.x);
        GlobalModelMatrix[3][1] = (float)((double)CMI->position.y - PlayerPos.y);
        GlobalModelMatrix[3][2] = (float)((double)CMI->position.z - PlayerPos.z);

        glm::mat4 MVP = VP * GlobalModelMatrix;

        glUniformMatrix4fv(waterShader.mvpID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(waterShader.mID, 1, GL_FALSE, &GlobalModelMatrix[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER, CMI->waterVboID);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LiquidVertex), 0);
        //uvs_texUnit_texIndex
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (12)));
        //color
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (16)));
        //light
        glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (20)));

        glDrawElements(GL_TRIANGLES, CMI->meshInfo.waterIndexSize, GL_UNSIGNED_INT, 0);
        GlobalModelMatrix[0][0] = 1.0;
        GlobalModelMatrix[1][1] = 1.0;
        GlobalModelMatrix[2][2] = 1.0;
    }
}

void ChunkRenderer::bindTransparentVao(ChunkMesh *CMI)
{
    if (CMI->transVaoID == 0) glGenVertexArrays(1, &(CMI->transVaoID));
    glBindVertexArray(CMI->transVaoID);

    glBindBuffer(GL_ARRAY_BUFFER, CMI->transVboID);

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

void ChunkRenderer::bindCutoutVao(ChunkMesh *CMI)
{
    if (CMI->cutoutVaoID == 0) glGenVertexArrays(1, &(CMI->cutoutVaoID));
    glBindVertexArray(CMI->cutoutVaoID);

    glBindBuffer(GL_ARRAY_BUFFER, CMI->cutoutVboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

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

void ChunkRenderer::bindVao(ChunkMesh *CMI)
{
    if (CMI->vaoID == 0) glGenVertexArrays(1, &(CMI->vaoID));
    glBindVertexArray(CMI->vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, CMI->vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

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