#include "stdafx.h"
#include "LiquidVoxelRenderStage.h"
#include "Camera.h"
#include "MeshManager.h"
#include "ChunkRenderer.h"

LiquidVoxelRenderStage::LiquidVoxelRenderStage(const GameRenderParams* gameRenderParams) :
    _gameRenderParams(gameRenderParams) {
    // Empty
}

void LiquidVoxelRenderStage::draw() {
    // Render water meshes
    ChunkRenderer::drawWater(_gameRenderParams);
}