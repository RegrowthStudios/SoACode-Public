#include "stdafx.h"
#include "OpaqueVoxelRenderStage.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/ShaderManager.h>
#include "Camera.h"
#include "Chunk.h"
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
    }
    m_program.use();
    m_program.enableVertexAttribArrays();

    glUniform1f(m_program.getUniform("lightType"), m_gameRenderParams->lightActive);

    glUniform3fv(m_program.getUniform("eyeNormalWorldspace"), 1, &(m_gameRenderParams->chunkCamera->getDirection()[0]));
    glUniform1f(m_program.getUniform("fogEnd"), m_gameRenderParams->fogEnd);
    glUniform1f(m_program.getUniform("fogStart"), m_gameRenderParams->fogStart);
    glUniform3fv(m_program.getUniform("fogColor"), 1, &(m_gameRenderParams->fogColor[0]));
    glUniform3fv(m_program.getUniform("lightPosition_worldspace"), 1, &(m_gameRenderParams->sunlightDirection[0]));
    glUniform1f(m_program.getUniform("specularExponent"), soaOptions.get(OPT_SPECULAR_EXPONENT).value.f);
    glUniform1f(m_program.getUniform("specularIntensity"), soaOptions.get(OPT_SPECULAR_INTENSITY).value.f * 0.3f);

    // Bind the block textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockPack.textureInfo.id);

    glUniform1f(m_program.getUniform("dt"), (GLfloat)bdt);
    glUniform1f(m_program.getUniform("sunVal"), m_gameRenderParams->sunlightIntensity);
    glUniform1f(m_program.getUniform("alphaMult"), 1.0f);

    f32 blockAmbient = 0.000f;
    glUniform3f(m_program.getUniform("ambientLight"), blockAmbient, blockAmbient, blockAmbient);
    glUniform3fv(m_program.getUniform("lightColor"), 1, &(m_gameRenderParams->sunlightColor[0]));

    glUniform1f(m_program.getUniform("fadeDistance"), ChunkRenderer::fadeDist);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    f64v3 closestPoint;
    static const f64v3 boxDims(CHUNK_WIDTH);
    static const f64v3 boxDims_2(CHUNK_WIDTH / 2);

    for (int i = chunkMeshes.size() - 1; i >= 0; i--) {
        ChunkMesh* cm = chunkMeshes[i];

        // Check for lazy deallocation
        if (cm->needsDestroy) {
            cmm->deleteMesh(cm, i);
            continue;
        }

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