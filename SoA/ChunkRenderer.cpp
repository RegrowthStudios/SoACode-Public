#include "stdafx.h"
#include "ChunkRenderer.h"

#include "Camera.h"
#include "Chunk.h"
#include "ChunkMeshManager.h"
#include "Frustum.h"
#include "GLProgramManager.h"
#include "GameManager.h"
#include "GameRenderParams.h"
#include "GeometrySorter.h"
#include "Options.h"
#include "PhysicsBlocks.h"
#include "RenderUtils.h"
#include "global.h"
#include "soaUtils.h"

const f32 sonarDistance = 200;
const f32 sonarWidth = 30;

#define CHUNK_DIAGONAL_LENGTH 28.0f

void ChunkRenderer::drawSonar(const GameRenderParams* gameRenderParams)
{
    ChunkMeshManager* cmm = gameRenderParams->chunkMeshmanager;
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    if (chunkMeshes.empty()) return;

    //*********************Blocks*******************
    
    vg::GLProgram* program = gameRenderParams->glProgramManager->getProgram("Sonar");
    program->use();

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    glUniform1f(program->getUniform("sonarDistance"), sonarDistance);
    glUniform1f(program->getUniform("waveWidth"), sonarWidth);
    glUniform1f(program->getUniform("dt"), sonarDt);

    glUniform1f(program->getUniform("fadeDistance"), fadeDist);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    for (unsigned int i = 0; i < chunkMeshes.size(); i++)
    {
        ChunkRenderer::drawChunkBlocks(chunkMeshes[i], program, 
                                       gameRenderParams->chunkCamera->getPosition(),
                                       gameRenderParams->chunkCamera->getViewProjectionMatrix());
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    program->unuse();
}

void ChunkRenderer::drawBlocks(const GameRenderParams* gameRenderParams)
{
    ChunkMeshManager* cmm = gameRenderParams->chunkMeshmanager;
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    if (chunkMeshes.empty()) return;

    const f64v3& position = gameRenderParams->chunkCamera->getPosition();

    vg::GLProgram* program = gameRenderParams->glProgramManager->getProgram("Block");
    program->use();

    glUniform1f(program->getUniform("lightType"), gameRenderParams->lightActive);

    glUniform3fv(program->getUniform("eyeNormalWorldspace"), 1, &(gameRenderParams->chunkCamera->getDirection()[0]));
    glUniform1f(program->getUniform("fogEnd"), gameRenderParams->fogEnd);
    glUniform1f(program->getUniform("fogStart"), gameRenderParams->fogStart);
    glUniform3fv(program->getUniform("fogColor"), 1, &(gameRenderParams->fogColor[0]));
    glUniform3fv(program->getUniform("lightPosition_worldspace"), 1, &(gameRenderParams->sunlightDirection[0]));
    glUniform1f(program->getUniform("specularExponent"), graphicsOptions.specularExponent);
    glUniform1f(program->getUniform("specularIntensity"), graphicsOptions.specularIntensity*0.3);

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.id);

    glUniform1f(program->getUniform("dt"), (GLfloat)bdt);
    glUniform1f(program->getUniform("sunVal"), gameRenderParams->sunlightIntensity);
    glUniform1f(program->getUniform("alphaMult"), 1.0f);

    f32 blockAmbient = 0.000f;
    glUniform3f(program->getUniform("ambientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3fv(program->getUniform("lightColor"), 1, &(gameRenderParams->sunlightColor[0]));

    glUniform1f(program->getUniform("fadeDistance"), fadeDist);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    f64v3 closestPoint;
    static const f64v3 boxDims(CHUNK_WIDTH);
    static const f64v3 boxDims_2(CHUNK_WIDTH / 2);

    for (int i = chunkMeshes.size() - 1; i >= 0; i--)
    {
        ChunkMesh* cm = chunkMeshes[i];

        // Check for lazy deallocation
        if (cm->needsDestroy) {
            cmm->deleteMesh(cm, i);
            continue;
        }

        if (gameRenderParams->chunkCamera->sphereInFrustum(f32v3(cm->position + boxDims_2 - position), CHUNK_DIAGONAL_LENGTH)) {
            // TODO(Ben): Implement perfect fade
            cm->inFrustum = 1;
            ChunkRenderer::drawChunkBlocks(cm, program, position,
                                           gameRenderParams->chunkCamera->getViewProjectionMatrix());
        } else{
            cm->inFrustum = 0;
        }
    }

    program->unuse();
}

void ChunkRenderer::drawCutoutBlocks(const GameRenderParams* gameRenderParams)
{
    ChunkMeshManager* cmm = gameRenderParams->chunkMeshmanager;
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    if (chunkMeshes.empty()) return;

    const f64v3& position = gameRenderParams->chunkCamera->getPosition();

    vg::GLProgram* program = gameRenderParams->glProgramManager->getProgram("Cutout");
    program->use();

    glUniform1f(program->getUniform("lightType"), gameRenderParams->lightActive);

    glUniform3fv(program->getUniform("eyeNormalWorldspace"), 1, &(gameRenderParams->chunkCamera->getDirection()[0]));
    glUniform1f(program->getUniform("fogEnd"), gameRenderParams->fogEnd);
    glUniform1f(program->getUniform("fogStart"), gameRenderParams->fogStart);
    glUniform3fv(program->getUniform("fogColor"), 1, &(gameRenderParams->fogColor[0]));
    glUniform3fv(program->getUniform("lightPosition_worldspace"), 1, &(gameRenderParams->sunlightDirection[0]));
    glUniform1f(program->getUniform("specularExponent"), graphicsOptions.specularExponent);
    glUniform1f(program->getUniform("alphaMult"), graphicsOptions.specularIntensity*0.3);

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.id);

    glUniform1f(program->getUniform("dt"), (GLfloat)bdt);

    glUniform1f(program->getUniform("sunVal"), gameRenderParams->sunlightIntensity);

    glUniform1f(program->getUniform("alphaMult"), 1.0f);

    float blockAmbient = 0.000f;
    glUniform3f(program->getUniform("ambientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3fv(program->getUniform("lightColor"), 1, &(gameRenderParams->sunlightColor[0]));

    glUniform1f(program->getUniform("fadeDistance"), fadeDist);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    glLineWidth(3);

    glDisable(GL_CULL_FACE);

    f64v3 cpos;

    static GLuint saveTicks = SDL_GetTicks();
    bool save = 0;
    if (SDL_GetTicks() - saveTicks >= 60000){ //save once per minute
        save = 1;
        saveTicks = SDL_GetTicks();
    }

    int mx, my, mz;
    double cx, cy, cz;
    double dx, dy, dz;
    mx = (int)position.x;
    my = (int)position.y;
    mz = (int)position.z;
    ChunkMesh *cm;

    for (int i = chunkMeshes.size() - 1; i >= 0; i--)
    {
        cm = chunkMeshes[i];

        if (cm->inFrustum){
            ChunkRenderer::drawChunkCutoutBlocks(cm, program, position,
                                                 gameRenderParams->chunkCamera->getViewProjectionMatrix());
        }
    }
    glEnable(GL_CULL_FACE);

    program->unuse();

}

void ChunkRenderer::drawTransparentBlocks(const GameRenderParams* gameRenderParams)
{
    ChunkMeshManager* cmm = gameRenderParams->chunkMeshmanager;
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    if (chunkMeshes.empty()) return;

    const f64v3& position = gameRenderParams->chunkCamera->getPosition();

    vg::GLProgram* program = gameRenderParams->glProgramManager->getProgram("Transparency");
    program->use();

    glUniform1f(program->getUniform("lightType"), gameRenderParams->lightActive);

    glUniform3fv(program->getUniform("eyeNormalWorldspace"), 1, &(gameRenderParams->chunkCamera->getDirection()[0]));
    glUniform1f(program->getUniform("fogEnd"), gameRenderParams->fogEnd);
    glUniform1f(program->getUniform("fogStart"), gameRenderParams->fogStart);
    glUniform3fv(program->getUniform("fogColor"), 1, &(gameRenderParams->fogColor[0]));
    glUniform3fv(program->getUniform("lightPosition_worldspace"), 1, &(gameRenderParams->sunlightDirection[0]));
    glUniform1f(program->getUniform("specularExponent"), graphicsOptions.specularExponent);
    glUniform1f(program->getUniform("specularIntensity"), graphicsOptions.specularIntensity*0.3);

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.id);

    glUniform1f(program->getUniform("dt"), (GLfloat)bdt);

    glUniform1f(program->getUniform("sunVal"), gameRenderParams->sunlightIntensity);

    glUniform1f(program->getUniform("alphaMult"), 1.0f);

    float blockAmbient = 0.000f;
    glUniform3f(program->getUniform("ambientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3fv(program->getUniform("lightColor"), 1, &(gameRenderParams->sunlightColor[0]));

    glUniform1f(program->getUniform("fadeDistance"), fadeDist);

    glLineWidth(3);

    glDisable(GL_CULL_FACE);

    f64v3 cpos;

    static GLuint saveTicks = SDL_GetTicks();
    bool save = 0;
    if (SDL_GetTicks() - saveTicks >= 60000){ //save once per minute
        save = 1;
        saveTicks = SDL_GetTicks();
    }


    ChunkMesh *cm;

    static i32v3 oldPos = i32v3(0);
    bool sort = false;

    i32v3 intPosition(fastFloor(position.x), fastFloor(position.y), fastFloor(position.z));

    if (oldPos != intPosition) {
        //sort the geometry
        sort = true;
        oldPos = intPosition;
    }

    for (int i = 0; i < chunkMeshes.size(); i++)
    {
        cm = chunkMeshes[i];
        if (sort) cm->needsSort = true;

        if (cm->inFrustum){

            if (cm->needsSort) {
                cm->needsSort = false;
                if (cm->transQuadIndices.size() != 0) {
                    GeometrySorter::sortTransparentBlocks(cm, intPosition);

                    //update index data buffer
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cm->transIndexID);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cm->transQuadIndices.size() * sizeof(ui32), NULL, GL_STATIC_DRAW);
                    void* v = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, cm->transQuadIndices.size() * sizeof(ui32), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);

                    if (v == NULL) pError("Failed to map sorted transparency buffer.");
                    memcpy(v, &(cm->transQuadIndices[0]), cm->transQuadIndices.size() * sizeof(ui32));
                    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
                }
            }

            ChunkRenderer::drawChunkTransparentBlocks(cm, program, position,
                                                      gameRenderParams->chunkCamera->getViewProjectionMatrix());
        }
    }
    glEnable(GL_CULL_FACE);

    program->unuse();

}

void ChunkRenderer::drawWater(const GameRenderParams* gameRenderParams)
{
    ChunkMeshManager* cmm = gameRenderParams->chunkMeshmanager;
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    if (chunkMeshes.empty()) return;

    vg::GLProgram* program = gameRenderParams->glProgramManager->getProgram("Water");
    program->use();

    glUniform1f(program->getUniform("sunVal"), gameRenderParams->sunlightIntensity);

    glUniform1f(program->getUniform("FogEnd"), gameRenderParams->fogEnd);
    glUniform1f(program->getUniform("FogStart"), gameRenderParams->fogStart);
    glUniform3fv(program->getUniform("FogColor"), 1, &(gameRenderParams->fogColor[0]));

    glUniform3fv(program->getUniform("LightPosition_worldspace"), 1, &(gameRenderParams->sunlightDirection[0]));

    if (NoChunkFade){
        glUniform1f(program->getUniform("FadeDistance"), (GLfloat)10000.0f);
    } else{
        glUniform1f(program->getUniform("FadeDistance"), (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f);
    }

    float blockAmbient = 0.000f;
    glUniform3f(program->getUniform("AmbientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3fv(program->getUniform("LightColor"), 1, &(gameRenderParams->sunlightColor[0]));

    glUniform1f(program->getUniform("dt"), (GLfloat)bdt);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, waterNormalTexture.id);
    glUniform1i(program->getUniform("normalMap"), 6);

    if (gameRenderParams->isUnderwater) glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    for (unsigned int i = 0; i < chunkMeshes.size(); i++) //they are sorted backwards??
    {
        ChunkRenderer::drawChunkWater(chunkMeshes[i], program,
                                      gameRenderParams->chunkCamera->getPosition(), 
                                      gameRenderParams->chunkCamera->getViewProjectionMatrix());
    }

    glDepthMask(GL_TRUE);
    if (gameRenderParams->isUnderwater) glEnable(GL_CULL_FACE);

    program->unuse();
}

void ChunkRenderer::drawChunkBlocks(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP)
{
    if (cm->vboID == 0) {
        //printf("VBO is 0 in drawChunkBlocks\n");
        return;
    }
    setMatrixTranslation(worldMatrix, f64v3(cm->position), PlayerPos);

    f32m4 MVP = VP * worldMatrix;

    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, &worldMatrix[0][0]);

    glBindVertexArray(cm->vaoID);

    const ChunkMeshRenderData& chunkMeshInfo = cm->meshInfo;
    
    //top
    if (chunkMeshInfo.pyVboSize){
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

void ChunkRenderer::drawChunkTransparentBlocks(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &playerPos, const f32m4 &VP) {
    if (cm->transVaoID == 0) return;

    setMatrixTranslation(worldMatrix, f64v3(cm->position), playerPos);

    f32m4 MVP = VP * worldMatrix;

    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, &worldMatrix[0][0]);

    glBindVertexArray(cm->transVaoID);

    glDrawElements(GL_TRIANGLES, cm->meshInfo.transVboSize, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

}

void ChunkRenderer::drawChunkCutoutBlocks(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &playerPos, const f32m4 &VP) {
    if (cm->cutoutVaoID == 0) return;

    setMatrixTranslation(worldMatrix, f64v3(cm->position), playerPos);

    f32m4 MVP = VP * worldMatrix;

    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, &worldMatrix[0][0]);

    glBindVertexArray(cm->cutoutVaoID);

    glDrawElements(GL_TRIANGLES, cm->meshInfo.cutoutVboSize, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

}

void ChunkRenderer::drawChunkWater(const ChunkMesh *cm, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP)
{
    //use drawWater bool to avoid checking frustum twice
    if (cm->inFrustum && cm->waterVboID){

        setMatrixTranslation(worldMatrix, f64v3(cm->position), PlayerPos);

        f32m4 MVP = VP * worldMatrix;

        glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, &worldMatrix[0][0]);

        glBindVertexArray(cm->waterVaoID);

        glDrawElements(GL_TRIANGLES, cm->meshInfo.waterIndexSize, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
}

void ChunkRenderer::bindTransparentVao(ChunkMesh *cm)
{
    if (cm->transVaoID == 0) glGenVertexArrays(1, &(cm->transVaoID));
    glBindVertexArray(cm->transVaoID);

    glBindBuffer(GL_ARRAY_BUFFER, cm->transVboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cm->transIndexID);

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

void ChunkRenderer::bindCutoutVao(ChunkMesh *cm)
{
    if (cm->cutoutVaoID == 0) glGenVertexArrays(1, &(cm->cutoutVaoID));
    glBindVertexArray(cm->cutoutVaoID);

    glBindBuffer(GL_ARRAY_BUFFER, cm->cutoutVboID);
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

void ChunkRenderer::bindVao(ChunkMesh *cm)
{
    if (cm->vaoID == 0) glGenVertexArrays(1, &(cm->vaoID));
    glBindVertexArray(cm->vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, cm->vboID);
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

void ChunkRenderer::bindWaterVao(ChunkMesh *cm)
{
    if (cm->waterVaoID == 0) glGenVertexArrays(1, &(cm->waterVaoID));
    glBindVertexArray(cm->waterVaoID);
    glBindBuffer(GL_ARRAY_BUFFER, cm->waterVboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, cm->waterVboID);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LiquidVertex), 0);
    //uvs_texUnit_texIndex
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (12)));
    //color
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (16)));
    //light
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (20)));

    glBindVertexArray(0);
}