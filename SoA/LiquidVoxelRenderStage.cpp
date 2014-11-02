#include "stdafx.h"
#include "LiquidVoxelRenderStage.h"
#include "Camera.h"
#include "MeshManager.h"
#include "ChunkRenderer.h"

LiquidVoxelRenderStage::LiquidVoxelRenderStage(const Camera* camera,
                                               const GameRenderParams* gameRenderParams,
                                               const MeshManager* meshManager) :
    IRenderStage(camera),
    _gameRenderParams(gameRenderParams),
    _meshManager(meshManager) {
    // Empty
}

void LiquidVoxelRenderStage::draw() {
    // Render water meshes
    ChunkRenderer::drawWater(_meshManager->getChunkMeshes(), _camera->projectionMatrix() * _camera->viewMatrix(), _gameRenderParams, _camera->position(), false);
}