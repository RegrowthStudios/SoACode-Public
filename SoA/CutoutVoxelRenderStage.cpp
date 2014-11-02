#include "stdafx.h"
#include "CutoutVoxelRenderStage.h"
#include "Camera.h"
#include "MeshManager.h"
#include "ChunkRenderer.h"


CutoutVoxelRenderStage::CutoutVoxelRenderStage(const Camera* camera,
                                               const GameRenderParams* gameRenderParams,
                                               const MeshManager* meshManager) :
    IRenderStage(camera),
    _gameRenderParams(gameRenderParams),
    _meshManager(meshManager) {
    // Empty
}


void CutoutVoxelRenderStage::draw()
{
    ChunkRenderer::drawCutoutBlocks(_meshManager->getChunkMeshes(), _camera->projectionMatrix() * _camera->viewMatrix(), _gameRenderParams, _camera->position(), _camera->direction());
}

