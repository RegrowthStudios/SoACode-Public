#include "stdafx.h"
#include "OpaqueVoxelRenderStage.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/ShaderManager.h>
#include "Camera.h"
#include "NChunk.h"
#include "BlockPack.h"
#include "ChunkMeshManager.h"
#include "ChunkRenderer.h"
#include "GameRenderParams.h"
#include "MeshManager.h"
#include "SoaOptions.h"
#include "RenderUtils.h"
#include "soaUtils.h"
#include "ShaderLoader.h"

void OpaqueVoxelRenderStage::hook(const GameRenderParams* gameRenderParams) {
    m_gameRenderParams = gameRenderParams;
}

void OpaqueVoxelRenderStage::render(const Camera* camera) {
    ChunkMeshManager* cmm = m_gameRenderParams->chunkMeshmanager;
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    if (chunkMeshes.empty()) return;

    const f64v3& position = m_gameRenderParams->chunkCamera->getPosition();

    if (!m_program.isCreated()) {
        m_program = ShaderLoader::createProgramFromFile("Shaders/BlockShading/standardShading.vert",
                                                             "Shaders/BlockShading/standardShading.frag");
        m_program.use();
        glUniform1i(m_program.getUniform("unTextures"), 0);
    }
    m_program.use();
    m_program.enableVertexAttribArrays();

    glUniform3fv(m_program.getUniform("unLightDirWorld"), 1, &(m_gameRenderParams->sunlightDirection[0]));
    glUniform1f(m_program.getUniform("unSpecularExponent"), soaOptions.get(OPT_SPECULAR_EXPONENT).value.f);
    glUniform1f(m_program.getUniform("unSpecularIntensity"), soaOptions.get(OPT_SPECULAR_INTENSITY).value.f * 0.3f);

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_gameRenderParams->blocks->texture.id);

    glUniform1f(m_program.getUniform("unSunVal"), m_gameRenderParams->sunlightIntensity);

    f32 blockAmbient = 0.000f;
    glUniform3f(m_program.getUniform("unAmbientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3fv(m_program.getUniform("unSunColor"), 1, &(m_gameRenderParams->sunlightColor[0]));

    glUniform1f(m_program.getUniform("unFadeDist"), 1000.0f/*ChunkRenderer::fadeDist*/);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NChunk::vboIndicesID);

    f64v3 closestPoint;
    static const f64v3 boxDims(CHUNK_WIDTH);
    static const f64v3 boxDims_2(CHUNK_WIDTH / 2);

    for (int i = chunkMeshes.size() - 1; i >= 0; i--) {
        ChunkMesh* cm = chunkMeshes[i];

        if (m_gameRenderParams->chunkCamera->sphereInFrustum(f32v3(cm->position + boxDims_2 - position), CHUNK_DIAGONAL_LENGTH)) {
            // TODO(Ben): Implement perfect fade
            cm->inFrustum = 1;
            ChunkRenderer::drawOpaque(cm, m_program, position,
                                           m_gameRenderParams->chunkCamera->getViewProjectionMatrix());
        } else {
            cm->inFrustum = 0;
        }
    }
    m_program.disableVertexAttribArrays();
    m_program.unuse();
}