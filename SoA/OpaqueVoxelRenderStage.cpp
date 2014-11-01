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


OpaqueVoxelRenderStage::~OpaqueVoxelRenderStage()
{
}

void OpaqueVoxelRenderStage::setState(vg::FrameBuffer* frameBuffer /*= nullptr*/)
{
    _inputFbo = frameBuffer;
}

void OpaqueVoxelRenderStage::draw()
{
    ChunkRenderer::drawBlocks(_meshManager->getChunkMeshes(), _camera->projectionMatrix() * _camera->viewMatrix(), _gameRenderParams, _camera->position(), _camera->direction());
}

bool OpaqueVoxelRenderStage::isVisible()
{
    throw std::logic_error("The method or operation is not implemented.");
}
