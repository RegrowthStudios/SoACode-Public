#include "stdafx.h"
#include "OpaqueVoxelRenderStage.h"
#include "Camera.h"
#include "MeshManager.h"
#include "ChunkRenderer.h"

OpaqueVoxelRenderStage::OpaqueVoxelRenderStage(Camera* camera, GameRenderParams* gameRenderParams, MeshManager* meshManager) :
    IRenderStage(camera),
    _gameRenderParams(gameRenderParams),
    _meshManager(meshManager)
{
}

void OpaqueVoxelRenderStage::draw()
{
    ChunkRenderer::drawBlocks(_meshManager->getChunkMeshes(), _camera->projectionMatrix() * _camera->viewMatrix(), _gameRenderParams, _camera->position(), _camera->direction());
}