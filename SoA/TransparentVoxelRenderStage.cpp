#include "stdafx.h"
#include "TransparentVoxelRenderStage.h"
#include "Camera.h"
#include "MeshManager.h"
#include "ChunkRenderer.h"

TransparentVoxelRenderStage::TransparentVoxelRenderStage(const Camera* camera, 
                                                         const GameRenderParams* gameRenderParams, 
                                                         const MeshManager* meshManager) :
    IRenderStage(camera),
    _gameRenderParams(gameRenderParams),
    _meshManager(meshManager) {
    // Empty
}

void TransparentVoxelRenderStage::draw() {
    glDepthMask(GL_FALSE);
    ChunkRenderer::drawTransparentBlocks(_meshManager->getChunkMeshes(), _camera->projectionMatrix() * _camera->viewMatrix(), _gameRenderParams, _camera->position(), _camera->direction());
    glDepthMask(GL_TRUE);
}

