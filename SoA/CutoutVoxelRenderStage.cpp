#include "stdafx.h"
#include "CutoutVoxelRenderStage.h"

#include <Vorb/graphics/GLProgram.h>
#include "Camera.h"
#include "Chunk.h"
#include "BlockPack.h"
#include "BlockTexturePack.h"
#include "ChunkMeshManager.h"
#include "ChunkRenderer.h"
#include "GameRenderParams.h"
#include "SoaOptions.h"
#include "RenderUtils.h"
#include "ShaderLoader.h"

void CutoutVoxelRenderStage::hook(ChunkRenderer* renderer, const GameRenderParams* gameRenderParams) {
    m_renderer = renderer;
    m_gameRenderParams = gameRenderParams;
}

void CutoutVoxelRenderStage::render(const Camera* camera VORB_MAYBE_UNUSED) {
    ChunkMeshManager* cmm = m_gameRenderParams->chunkMeshmanager;
    

    const f64v3& position = m_gameRenderParams->chunkCamera->getPosition();

    m_renderer->beginCutout(m_gameRenderParams->blockTexturePack->getAtlasTexture(), m_gameRenderParams->sunlightDirection,
                            m_gameRenderParams->sunlightColor);

    glDisable(GL_CULL_FACE);

    // f64v3 cpos;

    // TODO: Implement the saving mechanism/throw it out.
    // static GLuint saveTicks = SDL_GetTicks();
    // bool save = 0;
    // if (SDL_GetTicks() - saveTicks >= 60000) { //save once per minute
    //     save = 1;
    //     saveTicks = SDL_GetTicks();
    // }

    ChunkMesh *cm;

    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    {
        std::lock_guard<std::mutex> l(cmm->lckActiveChunkMeshes);
        if (chunkMeshes.empty()) return;
        for (int i = chunkMeshes.size() - 1; i >= 0; i--) {
            cm = chunkMeshes[i];

            if (cm->inFrustum) {
                m_renderer->drawCutout(cm, position,
                                       m_gameRenderParams->chunkCamera->getViewProjectionMatrix());
            }
        }
    }
    glEnable(GL_CULL_FACE);
    
    m_renderer->end();
}

