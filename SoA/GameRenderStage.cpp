#include "stdafx.h"
#include "GLProgramManager.h"
#include "GameRenderStage.h"
#include "FrameBuffer.h"
#include "GameManager.h"
#include "Planet.h"
#include "Camera.h"

GameRenderStage::GameRenderStage(vg::GLProgramManager* glProgramManager,
                                 vg::TextureCache* textureCache,
                                 Camera* chunkCamera,
                                 Camera* worldCamera) :
    _glProgramManager(glProgramManager),
    _textureCache(textureCache),
    _chunkCamera(chunkCamera),
    _worldCamera(worldCamera) {
    // Empty
}


GameRenderStage::~GameRenderStage() {
    // Empty
}

void GameRenderStage::render() {
  //  vg::FrameBuffer * fb = new vg::FrameBuffer();
    _renderTarget->bind();
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    f32m4 VP = _worldCamera->projectionMatrix() * _worldCamera->viewMatrix();
                                

}

void GameRenderStage::drawSpace(glm::mat4 &VP, bool connectedToPlanet) {
    f32m4 IMVP;

    if (connectedToPlanet) {
        // If we are connected to the planet, we need to rotate the skybox
        IMVP = VP * GameManager::planet->invRotationMatrix;
    } else {
        IMVP = VP;
    }

    glDepthMask(GL_FALSE);
   // _skyboxRenderer->drawSkybox(_glProgramManager->getProgram("Texture"), VP, 
 //   DrawSun((float)0, IMVP);
    glDepthMask(GL_TRUE);
}

void GameRenderStage::drawPlanets() {

}

void GameRenderStage::drawVoxels() {

}
