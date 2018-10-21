#include "stdafx.h"
#include "OpaqueVoxelRenderStage.h"

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/ShaderManager.h>
#include "Camera.h"
#include "Chunk.h"
#include "BlockPack.h"
#include "BlockTexturePack.h"
#include "ChunkMeshManager.h"
#include "ChunkRenderer.h"
#include "GameRenderParams.h"
#include "SoaOptions.h"
#include "RenderUtils.h"
#include "soaUtils.h"
#include "ShaderLoader.h"

void OpaqueVoxelRenderStage::hook(ChunkRenderer* renderer, const GameRenderParams* gameRenderParams) {
    m_gameRenderParams = gameRenderParams;
    m_renderer = renderer;
}

void OpaqueVoxelRenderStage::render(const Camera* camera VORB_MAYBE_UNUSED) {
    ChunkMeshManager* cmm = m_gameRenderParams->chunkMeshmanager;

    const f64v3& position = m_gameRenderParams->chunkCamera->getPosition();

    m_renderer->beginOpaque(m_gameRenderParams->blockTexturePack->getAtlasTexture(), m_gameRenderParams->sunlightDirection,
                            m_gameRenderParams->sunlightColor);
    
    // f64v3 closestPoint;
    // static const f64v3 boxDims(CHUNK_WIDTH);
    static const f64v3 boxDims_2(CHUNK_WIDTH / 2);
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    {
        std::lock_guard<std::mutex> l(cmm->lckActiveChunkMeshes);
        if (chunkMeshes.empty()) return;
        for (int i = chunkMeshes.size() - 1; i >= 0; i--) {
            ChunkMesh* cm = chunkMeshes[i];

            if (m_gameRenderParams->chunkCamera->sphereInFrustum(f32v3(cm->position + boxDims_2 - position), CHUNK_DIAGONAL_LENGTH)) {
                // TODO(Ben): Implement perfect fade
                cm->inFrustum = 1;
                m_renderer->drawOpaque(cm, position,
                                       m_gameRenderParams->chunkCamera->getViewProjectionMatrix());
            } else {
                cm->inFrustum = 0;
            }
        }
    }
    
    m_renderer->end();
}
