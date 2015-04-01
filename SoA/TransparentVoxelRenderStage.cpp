#include "stdafx.h"
#include "TransparentVoxelRenderStage.h"

#include <Vorb/graphics/GLProgram.h>
#include "Camera.h"
#include "Chunk.h"
#include "ChunkMeshManager.h"
#include "ChunkRenderer.h"
#include "GameRenderParams.h"
#include "GeometrySorter.h"
#include "MeshManager.h"
#include "Options.h"
#include "RenderUtils.h"
#include "ShaderLoader.h"

TransparentVoxelRenderStage::TransparentVoxelRenderStage(const GameRenderParams* gameRenderParams) :
    m_gameRenderParams(gameRenderParams) {
    // Empty
}

void TransparentVoxelRenderStage::render() {
    glDepthMask(GL_FALSE);
    ChunkMeshManager* cmm = m_gameRenderParams->chunkMeshmanager;
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    if (chunkMeshes.empty()) return;

    const f64v3& position = m_gameRenderParams->chunkCamera->getPosition();

    if (!m_program) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/BlockShading/standardShading.vert",
                                                             "Shaders/BlockShading/cutoutShading.frag");
    }
    m_program->use();
    m_program->enableVertexAttribArrays();

    glUniform1f(m_program->getUniform("lightType"), m_gameRenderParams->lightActive);

    glUniform3fv(m_program->getUniform("eyeNormalWorldspace"), 1, &(m_gameRenderParams->chunkCamera->getDirection()[0]));
    glUniform1f(m_program->getUniform("fogEnd"), m_gameRenderParams->fogEnd);
    glUniform1f(m_program->getUniform("fogStart"), m_gameRenderParams->fogStart);
    glUniform3fv(m_program->getUniform("fogColor"), 1, &(m_gameRenderParams->fogColor[0]));
    glUniform3fv(m_program->getUniform("lightPosition_worldspace"), 1, &(m_gameRenderParams->sunlightDirection[0]));
    glUniform1f(m_program->getUniform("specularExponent"), graphicsOptions.specularExponent);
    glUniform1f(m_program->getUniform("specularIntensity"), graphicsOptions.specularIntensity*0.3);

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.id);

    glUniform1f(m_program->getUniform("dt"), (GLfloat)bdt);

    glUniform1f(m_program->getUniform("sunVal"), m_gameRenderParams->sunlightIntensity);

    glUniform1f(m_program->getUniform("alphaMult"), 1.0f);

    float blockAmbient = 0.000f;
    glUniform3f(m_program->getUniform("ambientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3fv(m_program->getUniform("lightColor"), 1, &(m_gameRenderParams->sunlightColor[0]));

    glUniform1f(m_program->getUniform("fadeDistance"), ChunkRenderer::fadeDist);

    glLineWidth(3);

    glDisable(GL_CULL_FACE);

    f64v3 cpos;

    static GLuint saveTicks = SDL_GetTicks();
    bool save = 0;
    if (SDL_GetTicks() - saveTicks >= 60000) { //save once per minute
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

    for (int i = 0; i < chunkMeshes.size(); i++) {
        cm = chunkMeshes[i];
        if (sort) cm->needsSort = true;

        if (cm->inFrustum) {

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

            ChunkRenderer::drawTransparent(cm, m_program, position,
                                                      m_gameRenderParams->chunkCamera->getViewProjectionMatrix());
        }
    }
    glEnable(GL_CULL_FACE);

    m_program->disableVertexAttribArrays();
    m_program->unuse();
    glDepthMask(GL_TRUE);
}

