#include "stdafx.h"
#include "CutoutVoxelRenderStage.h"
#include "Camera.h"
#include "MeshManager.h"
#include "ChunkRenderer.h"


CutoutVoxelRenderStage::CutoutVoxelRenderStage(Camera* camera, GameRenderParams* gameRenderParams, MeshManager* meshManager) :
    IRenderStage(camera),
    _gameRenderParams(gameRenderParams),
    _meshManager(meshManager)
{
}


CutoutVoxelRenderStage::~CutoutVoxelRenderStage()
{
}

void CutoutVoxelRenderStage::setState(vg::FrameBuffer* frameBuffer /*= nullptr*/)
{
    throw std::logic_error("The method or operation is not implemented.");
}

void CutoutVoxelRenderStage::draw()
{
    ChunkRenderer::drawCutoutBlocks(_meshManager->getChunkMeshes(), _camera->projectionMatrix() * _camera->viewMatrix(), _gameRenderParams, _camera->position(), _camera->direction());
}

bool CutoutVoxelRenderStage::isVisible()
{
    throw std::logic_error("The method or operation is not implemented.");
}
