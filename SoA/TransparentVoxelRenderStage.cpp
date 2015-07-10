#include "stdafx.h"
#include "TransparentVoxelRenderStage.h"

#include <Vorb/graphics/GLProgram.h>
#include "BlockPack.h"
#include "BlockTexturePack.h"
#include "Camera.h"
#include "ChunkMeshManager.h"
#include "ChunkRenderer.h"
#include "GameRenderParams.h"
#include "GeometrySorter.h"
#include "MeshManager.h"
#include "Chunk.h"
#include "RenderUtils.h"
#include "ShaderLoader.h"
#include "SoaOptions.h"

void TransparentVoxelRenderStage::hook(ChunkRenderer* renderer, const GameRenderParams* gameRenderParams) {
    m_renderer = renderer;
    m_gameRenderParams = gameRenderParams;
}

void TransparentVoxelRenderStage::render(const Camera* camera) {
    glDepthMask(GL_FALSE);
    ChunkMeshManager* cmm = m_gameRenderParams->chunkMeshmanager;
    const std::vector <ChunkMesh *>& chunkMeshes = cmm->getChunkMeshes();
    if (chunkMeshes.empty()) return;

    const f64v3& position = m_gameRenderParams->chunkCamera->getPosition();

    m_renderer->beginTransparent(m_gameRenderParams->blockTexturePack->getAtlasTexture(), m_gameRenderParams->sunlightDirection,
                                 m_gameRenderParams->sunlightColor);

    glDisable(GL_CULL_FACE);

    f64v3 cpos;

    static i32v3 oldPos = i32v3(0);
    bool sort = false;

    i32v3 intPosition(fastFloor(position.x), fastFloor(position.y), fastFloor(position.z));

    if (oldPos != intPosition) {
        //sort the geometry
        sort = true;
        oldPos = intPosition;
    }

    for (size_t i = 0; i < chunkMeshes.size(); i++) {
        ChunkMesh* cm = chunkMeshes[i];
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

            m_renderer->drawTransparent(cm, position,
                                        m_gameRenderParams->chunkCamera->getViewProjectionMatrix());
        }
    }
    glEnable(GL_CULL_FACE);

    m_renderer->end();
    glDepthMask(GL_TRUE);
}

