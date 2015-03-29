#include "stdafx.h"
#include "CutoutVoxelRenderStage.h"

#include "Camera.h"
#include "ChunkRenderer.h"
#include "MeshManager.h"

CutoutVoxelRenderStage::CutoutVoxelRenderStage(const GameRenderParams* gameRenderParams) :
    _gameRenderParams(gameRenderParams) {
    // Empty
}


void CutoutVoxelRenderStage::render() {
    ChunkRenderer::drawCutoutBlocks(_gameRenderParams);
}

