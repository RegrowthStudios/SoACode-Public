#include "stdafx.h"
#include "SonarRenderStage.h"

#include <Vorb/graphics/GLProgram.h>
#include "Camera.h"
#include "Chunk.h"
#include "ChunkMeshManager.h"
#include "ChunkRenderer.h"
#include "GameRenderParams.h"
#include "MeshManager.h"
#include "SoaOptions.h"
#include "RenderUtils.h"
#include "ShaderLoader.h"

// TODO(Ben): Don't hardcode
const f32 SONAR_DISTANCE = 200.0f;
const f32 SONAR_WIDTH = 30.0f;

SonarRenderStage::SonarRenderStage(const GameRenderParams* gameRenderParams) :
    m_gameRenderParams(gameRenderParams) {
    // Empty
}

void SonarRenderStage::render(const Camera* camera) {
    glDisable(GL_DEPTH_TEST);
    ChunkMeshManager* cmm = m_gameRenderParams->chunkMeshmanager;
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    if (chunkMeshes.empty()) return;

    if (!m_program.isCreated()) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/BlockShading/standardShading.vert",
                                                             "Shaders/BlockShading/sonarShading.frag");
    }
    m_program.use();
    m_program.enableVertexAttribArrays();

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_gameRenderParams->blocks->texture.id);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    glUniform1f(m_program.getUniform("sonarDistance"), SONAR_DISTANCE);
    glUniform1f(m_program.getUniform("waveWidth"), SONAR_WIDTH);
    glUniform1f(m_program.getUniform("dt"), 1.0f);

    glUniform1f(m_program.getUniform("fadeDistance"), ChunkRenderer::fadeDist);

    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    for (unsigned int i = 0; i < chunkMeshes.size(); i++) {
        ChunkRenderer::drawOpaque(chunkMeshes[i], m_program,
                                       m_gameRenderParams->chunkCamera->getPosition(),
                                       m_gameRenderParams->chunkCamera->getViewProjectionMatrix());
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    m_program.unuse();
    glEnable(GL_DEPTH_TEST);
}