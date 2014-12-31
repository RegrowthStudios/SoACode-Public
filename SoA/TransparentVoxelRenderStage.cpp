#include "stdafx.h"
#include "TransparentVoxelRenderStage.h"

#include "Camera.h"
#include "MeshManager.h"
#include "ChunkRenderer.h"

TransparentVoxelRenderStage::TransparentVoxelRenderStage(const GameRenderParams* gameRenderParams) :
    _gameRenderParams(gameRenderParams) {
    // Empty
}

void TransparentVoxelRenderStage::draw() {
    glDepthMask(GL_FALSE);
    ChunkRenderer::drawTransparentBlocks(_gameRenderParams);
    glDepthMask(GL_TRUE);
}

