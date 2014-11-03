#include "stdafx.h"
#include "Camera.h"
#include "ChunkRenderer.h"
#include "CutoutVoxelRenderStage.h"
#include "MeshManager.h"


CutoutVoxelRenderStage::CutoutVoxelRenderStage(const GameRenderParams* gameRenderParams) :
    _gameRenderParams(gameRenderParams) {
    // Empty
}


void CutoutVoxelRenderStage::draw()
{
    ChunkRenderer::drawCutoutBlocks(_gameRenderParams);
}

