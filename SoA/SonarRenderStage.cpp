#include "stdafx.h"
#include "SonarRenderStage.h"
#include "ChunkRenderer.h"
#include "Camera.h"
#include "MeshManager.h"


SonarRenderStage::SonarRenderStage(const Camera* camera, 
                                   const MeshManager* meshManager) :
    IRenderStage(camera),
    _meshManager(meshManager) {
    // Empty
}

void SonarRenderStage::draw()
{
    glDisable(GL_DEPTH_TEST);
    ChunkRenderer::drawSonar(_meshManager->getChunkMeshes(), _camera->projectionMatrix() * _camera->viewMatrix(), _camera->position());
    glEnable(GL_DEPTH_TEST);
}