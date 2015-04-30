#include "stdafx.h"
#include "LiquidVoxelRenderStage.h"

#include <Vorb/graphics/GLProgram.h>
#include "Camera.h"
#include "Chunk.h"
#include "ChunkMeshManager.h"
#include "ChunkRenderer.h"
#include "GameRenderParams.h"
#include "MeshManager.h"
#include "Options.h"
#include "RenderUtils.h"
#include "ShaderLoader.h"

LiquidVoxelRenderStage::LiquidVoxelRenderStage(const GameRenderParams* gameRenderParams) :
    m_gameRenderParams(gameRenderParams) {
    // Empty
}

void LiquidVoxelRenderStage::render() {
    ChunkMeshManager* cmm = m_gameRenderParams->chunkMeshmanager;
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    if (chunkMeshes.empty()) return;

    if (!m_program) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/WaterShading/WaterShading.vert",
                                                             "Shaders/WaterShading/WaterShading.frag");
    }
    m_program->use();
    m_program->enableVertexAttribArrays();

    glUniform1f(m_program->getUniform("sunVal"), m_gameRenderParams->sunlightIntensity);

    glUniform1f(m_program->getUniform("FogEnd"), m_gameRenderParams->fogEnd);
    glUniform1f(m_program->getUniform("FogStart"), m_gameRenderParams->fogStart);
    glUniform3fv(m_program->getUniform("FogColor"), 1, &(m_gameRenderParams->fogColor[0]));

    glUniform3fv(m_program->getUniform("LightPosition_worldspace"), 1, &(m_gameRenderParams->sunlightDirection[0]));

    if (NoChunkFade) {
        glUniform1f(m_program->getUniform("FadeDistance"), (GLfloat)10000.0f);
    } else {
        glUniform1f(m_program->getUniform("FadeDistance"), (GLfloat)graphicsOptions.voxelRenderDistance - 12.5f);
    }

    float blockAmbient = 0.000f;
    glUniform3f(m_program->getUniform("AmbientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3fv(m_program->getUniform("LightColor"), 1, &(m_gameRenderParams->sunlightColor[0]));

    glUniform1f(m_program->getUniform("dt"), (GLfloat)bdt);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, waterNormalTexture.id);
    glUniform1i(m_program->getUniform("normalMap"), 6);

    if (m_gameRenderParams->isUnderwater) glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    for (unsigned int i = 0; i < chunkMeshes.size(); i++) //they are sorted backwards??
    {
        ChunkRenderer::drawWater(chunkMeshes[i], m_program,
                          m_gameRenderParams->chunkCamera->getPosition(),
                          m_gameRenderParams->chunkCamera->getViewProjectionMatrix());
    }

    glDepthMask(GL_TRUE);
    if (m_gameRenderParams->isUnderwater) glEnable(GL_CULL_FACE);

    m_program->disableVertexAttribArrays();
    m_program->unuse();
}
