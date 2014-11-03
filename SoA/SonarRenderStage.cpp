#include "stdafx.h"

#include "Camera.h"
#include "ChunkRenderer.h"
#include "MeshManager.h"
#include "SonarRenderStage.h"


SonarRenderStage::SonarRenderStage(const GameRenderParams* gameRenderParams) :
    _gameRenderParams(gameRenderParams) {
    // Empty
}

void SonarRenderStage::draw()
{
    glDisable(GL_DEPTH_TEST);
    ChunkRenderer::drawSonar(_gameRenderParams);
    glEnable(GL_DEPTH_TEST);
}