#include "stdafx.h"
#include "LiquidVoxelRenderStage.h"

#include <Vorb/graphics/GLProgram.h>
#include "BlockTexturePack.h"
#include "Camera.h"
#include "Chunk.h"
#include "ChunkMeshManager.h"
#include "ChunkRenderer.h"
#include "GameRenderParams.h"
#include "MeshManager.h"
#include "RenderUtils.h"
#include "ShaderLoader.h"
#include "SoaOptions.h"

void LiquidVoxelRenderStage::hook(ChunkRenderer* renderer, const GameRenderParams* gameRenderParams) {
    m_renderer = renderer;
    m_gameRenderParams = gameRenderParams;
}

void LiquidVoxelRenderStage::render(const Camera* camera) {
    ChunkMeshManager* cmm = m_gameRenderParams->chunkMeshmanager;
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    if (chunkMeshes.empty()) return;
   
    m_renderer->beginLiquid(m_gameRenderParams->blockTexturePack->getAtlasTexture(), m_gameRenderParams->sunlightDirection,
                            m_gameRenderParams->sunlightColor);

    if (m_gameRenderParams->isUnderwater) glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Chunk::vboIndicesID);

    for (unsigned int i = 0; i < chunkMeshes.size(); i++) //they are sorted backwards??
    {
        m_renderer->drawLiquid(chunkMeshes[i],
                          m_gameRenderParams->chunkCamera->getPosition(),
                          m_gameRenderParams->chunkCamera->getViewProjectionMatrix());
    }

    glDepthMask(GL_TRUE);
    if (m_gameRenderParams->isUnderwater) glEnable(GL_CULL_FACE);

    m_renderer->end();
}
