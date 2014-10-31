#include "stdafx.h"
#include "TransparentVoxelRenderStage.h"
#include "Camera.h"
#include "MeshManager.h"
#include "ChunkRenderer.h"

TransparentVoxelRenderStage::TransparentVoxelRenderStage(Camera* camera, GameRenderParams* gameRenderParams, MeshManager* meshManager) :
    IRenderStage(camera),
    _gameRenderParams(gameRenderParams),
    _meshManager(meshManager)
{
}

TransparentVoxelRenderStage::~TransparentVoxelRenderStage()
{
}

void TransparentVoxelRenderStage::setState(vg::FrameBuffer* frameBuffer /*= nullptr*/) {

}

void TransparentVoxelRenderStage::draw() {
    glDepthMask(GL_FALSE);
    ChunkRenderer::drawTransparentBlocks(_meshManager->getChunkMeshes(), _camera->projectionMatrix() * _camera->viewMatrix(), _gameRenderParams, _camera->position(), _camera->direction());
    glDepthMask(GL_TRUE);
}

bool TransparentVoxelRenderStage::isVisible() {

}

