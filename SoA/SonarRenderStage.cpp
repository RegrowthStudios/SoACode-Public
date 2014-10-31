#include "stdafx.h"
#include "SonarRenderStage.h"
#include "ChunkRenderer.h"
#include "Camera.h"
#include "MeshManager.h"


SonarRenderStage::SonarRenderStage(Camera* camera, MeshManager* meshManager) :
    IRenderStage(camera),
    _meshManager(meshManager)
{
}


SonarRenderStage::~SonarRenderStage()
{
}

void SonarRenderStage::draw()
{
    glDisable(GL_DEPTH_TEST);
    ChunkRenderer::drawSonar(_meshManager->getChunkMeshes(), _camera->projectionMatrix() * _camera->viewMatrix(), _camera->position());
    glEnable(GL_DEPTH_TEST);
}

void SonarRenderStage::setState(vg::FrameBuffer* frameBuffer /*= nullptr*/)
{
    throw std::logic_error("The method or operation is not implemented.");
}

bool SonarRenderStage::isVisible()
{
    throw std::logic_error("The method or operation is not implemented.");
}
