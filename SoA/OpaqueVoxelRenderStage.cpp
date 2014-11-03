#include "stdafx.h"
#include "OpaqueVoxelRenderStage.h"
#include "Camera.h"
#include "MeshManager.h"
#include "ChunkRenderer.h"

OpaqueVoxelRenderStage::OpaqueVoxelRenderStage(const GameRenderParams* gameRenderParams) :
    _gameRenderParams(gameRenderParams)
{
    // Empty
}

void OpaqueVoxelRenderStage::draw()
{
    ChunkRenderer::drawBlocks(_gameRenderParams);
}