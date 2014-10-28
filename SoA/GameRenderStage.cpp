#include "stdafx.h"
#include "GameRenderStage.h"
#include "FrameBuffer.h"
#include "GameManager.h"
#include "Planet.h"

GameRenderStage::GameRenderStage() {
    // Empty
}


GameRenderStage::~GameRenderStage() {
    // Empty
}

void GameRenderStage::render() {
    vg::FrameBuffer * fb = new vg::FrameBuffer();
    _renderTarget->bind();
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GameRenderStage::drawSpace(glm::mat4 &VP, bool connectedToPlanet) {
    glm::mat4 IMVP;

    if (connectedToPlanet) {
        // If we are connected to the planet, we need to rotate the skybox
        IMVP = VP * GameManager::planet->invRotationMatrix;
    } else {
        IMVP = VP;
    }

    glDepthMask(GL_FALSE);
    if (!drawMode) DrawStars((float)0, IMVP);
    DrawSun((float)0, IMVP);
    glDepthMask(GL_TRUE);
}

void GameRenderStage::drawPlanets() {

}

void GameRenderStage::drawVoxels() {

}
