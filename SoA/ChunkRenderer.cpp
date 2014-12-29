#include "stdafx.h"
#include "ChunkRenderer.h"

#include "Camera.h"
#include "Chunk.h"
#include "Frustum.h"
#include "GLProgramManager.h"
#include "GameManager.h"
#include "GameRenderParams.h"
#include "GeometrySorter.h"
#include "Options.h"
#include "PhysicsBlocks.h"
#include "global.h"

const float sonarDistance = 200;
const float sonarWidth = 30;

void ChunkRenderer::drawSonar(const GameRenderParams* gameRenderParams)
{
    //*********************Blocks*******************

    vg::GLProgram* program = GameManager::glProgramManager->getProgram("Sonar");
    program->use();

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    glUniform1f(program->getUniform("sonarDistance"), sonarDistance);
    glUniform1f(program->getUniform("waveWidth"), sonarWidth);
    glUniform1f(program->getUniform("dt"), sonarDt);

    float fadeDist;
    if (NoChunkFade){
        fadeDist = (GLfloat)10000.0f;
    } else{
        fadeDist = (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f;
    }
    glUniform1f(program->getUniform("fadeDistance"), fadeDist);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    const std::vector <ChunkMesh *>& chunkMeshes = *(gameRenderParams->chunkMeshes);

    for (unsigned int i = 0; i < chunkMeshes.size(); i++)
    {
        ChunkRenderer::drawChunkBlocks(chunkMeshes[i], program, 
                                       gameRenderParams->chunkCamera->getPosition(),
                                       gameRenderParams->VP);
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    program->unuse();
}

void ChunkRenderer::drawBlocks(const GameRenderParams* gameRenderParams)
{
    const f64v3& position = gameRenderParams->chunkCamera->getPosition();

    vg::GLProgram* program = GameManager::glProgramManager->getProgram("Block");
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

    float fadeDist;
    if (NoChunkFade){
        fadeDist = (GLfloat)10000.0f;
    } else{
        fadeDist = (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f;
    }

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

    const std::vector <ChunkMesh *>& chunkMeshes = *(gameRenderParams->chunkMeshes);

    for (int i = chunkMeshes.size() - 1; i >= 0; i--)
    {
        cm = chunkMeshes[i];
        const glm::ivec3 &cmPos = cm->position;

        //calculate distance
        cx = (mx <= cmPos.x) ? cmPos.x : ((mx > cmPos.x + CHUNK_WIDTH) ? (cmPos.x + CHUNK_WIDTH) : mx);
        cy = (my <= cmPos.y) ? cmPos.y : ((my > cmPos.y + CHUNK_WIDTH) ? (cmPos.y + CHUNK_WIDTH) : my);
        cz = (mz <= cmPos.z) ? cmPos.z : ((mz > cmPos.z + CHUNK_WIDTH) ? (cmPos.z + CHUNK_WIDTH) : mz);
        dx = cx - mx;
        dy = cy - my;
        dz = cz - mz;
        cm->distance = sqrt(dx*dx + dy*dy + dz*dz);

        if (gameRenderParams->chunkCamera->sphereInFrustum(f32v3(f64v3(cmPos) + f64v3(CHUNK_WIDTH / 2) - position), 28.0f)) {
            if (cm->distance < fadeDist + 12.5){
                cm->inFrustum = 1;
                ChunkRenderer::drawChunkBlocks(cm, program, position, gameRenderParams->VP);
            } else{
                cm->inFrustum = 0;
            }
        } else{
            cm->inFrustum = 0;
        }
    }
    glEnable(GL_CULL_FACE);

    program->unuse();
}

void ChunkRenderer::drawCutoutBlocks(const GameRenderParams* gameRenderParams)
{
    const f64v3& position = gameRenderParams->chunkCamera->getPosition();

    vg::GLProgram* program = GameManager::glProgramManager->getProgram("Cutout");
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

    float fadeDist;
    if (NoChunkFade){
        fadeDist = (GLfloat)10000.0f;
    } else{
        fadeDist = (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f;
    }

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

    const std::vector <ChunkMesh *>& chunkMeshes = *(gameRenderParams->chunkMeshes);

    for (int i = chunkMeshes.size() - 1; i >= 0; i--)
    {
        cm = chunkMeshes[i];

        if (cm->inFrustum){
            ChunkRenderer::drawChunkCutoutBlocks(cm, program, position, gameRenderParams->VP);
        }
    }
    glEnable(GL_CULL_FACE);

    program->unuse();

}

void ChunkRenderer::drawTransparentBlocks(const GameRenderParams* gameRenderParams)
{
    const f64v3& position = gameRenderParams->chunkCamera->getPosition();

    vg::GLProgram* program = GameManager::glProgramManager->getProgram("Transparency");
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

    float fadeDist;
    if (NoChunkFade){
        fadeDist = (GLfloat)10000.0f;
    } else{
        fadeDist = (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f;
    }

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

    const std::vector <ChunkMesh *>& chunkMeshes = *(gameRenderParams->chunkMeshes);

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

            ChunkRenderer::drawChunkTransparentBlocks(cm, program, position, gameRenderParams->VP);
        }
    }
    glEnable(GL_CULL_FACE);

    program->unuse();

}

void ChunkRenderer::drawWater(const GameRenderParams* gameRenderParams)
{
    vg::GLProgram* program = GameManager::glProgramManager->getProgram("Water");
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

    const std::vector <ChunkMesh *>& chunkMeshes = *(gameRenderParams->chunkMeshes);

    for (unsigned int i = 0; i < chunkMeshes.size(); i++) //they are sorted backwards??
    {
        ChunkRenderer::drawChunkWater(chunkMeshes[i], program,
                                      gameRenderParams->chunkCamera->getPosition(), 
                                      gameRenderParams->VP);
    }

    glDepthMask(GL_TRUE);
    if (gameRenderParams->isUnderwater) glEnable(GL_CULL_FACE);

    program->unuse();
}

void ChunkRenderer::drawChunkBlocks(const ChunkMesh *CMI, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP)
{
    if (CMI->vboID == 0) return;

    GlobalModelMatrix[3][0] = ((float)((double)CMI->position.x - PlayerPos.x));
    GlobalModelMatrix[3][1] = ((float)((double)CMI->position.y - PlayerPos.y));
    GlobalModelMatrix[3][2] = ((float)((double)CMI->position.z - PlayerPos.z));

    f32m4 MVP = VP * GlobalModelMatrix;

    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, &GlobalModelMatrix[0][0]);

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

void ChunkRenderer::drawChunkTransparentBlocks(const ChunkMesh *CMI, const vg::GLProgram* program, const f64v3 &playerPos, const f32m4 &VP) {
    if (CMI->transVaoID == 0) return;

    GlobalModelMatrix[3][0] = ((float)((double)CMI->position.x - playerPos.x));
    GlobalModelMatrix[3][1] = ((float)((double)CMI->position.y - playerPos.y));
    GlobalModelMatrix[3][2] = ((float)((double)CMI->position.z - playerPos.z));

    f32m4 MVP = VP * GlobalModelMatrix;

    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, &GlobalModelMatrix[0][0]);

    glBindVertexArray(CMI->transVaoID);

    glDrawElements(GL_TRIANGLES, CMI->meshInfo.transVboSize, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

}

void ChunkRenderer::drawChunkCutoutBlocks(const ChunkMesh *CMI, const vg::GLProgram* program, const f64v3 &playerPos, const f32m4 &VP) {
    if (CMI->cutoutVaoID == 0) return;

    GlobalModelMatrix[3][0] = ((float)((double)CMI->position.x - playerPos.x));
    GlobalModelMatrix[3][1] = ((float)((double)CMI->position.y - playerPos.y));
    GlobalModelMatrix[3][2] = ((float)((double)CMI->position.z - playerPos.z));

    f32m4 MVP = VP * GlobalModelMatrix;

    glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, &GlobalModelMatrix[0][0]);

    glBindVertexArray(CMI->cutoutVaoID);

    glDrawElements(GL_TRIANGLES, CMI->meshInfo.cutoutVboSize, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);

}

void ChunkRenderer::drawChunkWater(const ChunkMesh *CMI, const vg::GLProgram* program, const f64v3 &PlayerPos, const f32m4 &VP)
{
    //use drawWater bool to avoid checking frustum twice
    if (CMI->inFrustum && CMI->waterVboID){

        GlobalModelMatrix[3][0] = (float)((double)CMI->position.x - PlayerPos.x);
        GlobalModelMatrix[3][1] = (float)((double)CMI->position.y - PlayerPos.y);
        GlobalModelMatrix[3][2] = (float)((double)CMI->position.z - PlayerPos.z);

        f32m4 MVP = VP * GlobalModelMatrix;

        glUniformMatrix4fv(program->getUniform("MVP"), 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(program->getUniform("M"), 1, GL_FALSE, &GlobalModelMatrix[0][0]);

        glBindVertexArray(CMI->waterVaoID);

        glDrawElements(GL_TRIANGLES, CMI->meshInfo.waterIndexSize, GL_UNSIGNED_INT, 0);
        GlobalModelMatrix[0][0] = 1.0;
        GlobalModelMatrix[1][1] = 1.0;
        GlobalModelMatrix[2][2] = 1.0;

        glBindVertexArray(0);
    }
}

void ChunkRenderer::bindTransparentVao(ChunkMesh *CMI)
{
    if (CMI->transVaoID == 0) glGenVertexArrays(1, &(CMI->transVaoID));
    glBindVertexArray(CMI->transVaoID);

    glBindBuffer(GL_ARRAY_BUFFER, CMI->transVboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CMI->transIndexID);

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

void ChunkRenderer::bindWaterVao(ChunkMesh *CMI)
{
    if (CMI->waterVaoID == 0) glGenVertexArrays(1, &(CMI->waterVaoID));
    glBindVertexArray(CMI->waterVaoID);
    glBindBuffer(GL_ARRAY_BUFFER, CMI->waterVboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, CMI->waterVboID);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LiquidVertex), 0);
    //uvs_texUnit_texIndex
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (12)));
    //color
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (16)));
    //light
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(LiquidVertex), ((char *)NULL + (20)));

    glBindVertexArray(0);
}