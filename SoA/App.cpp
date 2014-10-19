#include "stdafx.h"
#include "App.h"

#include "InitScreen.h"
#include "LoadScreen.h"
#include "MainMenuScreen.h"
#include "GamePlayScreen.h"
#include "ScreenList.h"
#include "FrameBuffer.h"

void App::addScreens() {
    scrInit = new InitScreen(this);
    scrLoad = new LoadScreen(this);
    scrMainMenu = new MainMenuScreen(this);
    scrGamePlay = new GamePlayScreen(this);

    _screenList->addScreen(scrInit);
    _screenList->addScreen(scrLoad);
    _screenList->addScreen(scrMainMenu);
    _screenList->addScreen(scrGamePlay);

    _screenList->setScreen(scrInit->getIndex());
}

void App::onInit() {
    
    // Load the graphical options
    initializeOptions();
    loadOptions();

    if (graphicsOptions.msaa > 0){
        glEnable(GL_MULTISAMPLE);
        frameBuffer = new FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT, graphicsOptions.screenWidth, graphicsOptions.screenHeight, graphicsOptions.msaa);
    } else{
        glDisable(GL_MULTISAMPLE);
        frameBuffer = new FrameBuffer(GL_RGBA16F, GL_HALF_FLOAT, graphicsOptions.screenWidth, graphicsOptions.screenHeight);
    }
}

void App::onExit() {
    // Empty
}

App::~App() {
    if (scrInit) {
        delete scrInit;
        scrInit = nullptr;
    }

    if (scrLoad) {
        delete scrLoad;
        scrLoad = nullptr;
    }
}

void App::drawFrameBuffer(const f32m4& VP) const {

    /// Used for screen space motion blur
    static bool start = 1;

    vcore::GLProgram* program;

    if (graphicsOptions.motionBlur > 0){
        program = GameManager::glProgramManager->getProgram("MotionBlur");
        program->use();
        program->enableVertexAttribArrays();

        static f32m4 currMat;
        static f32m4 oldVP;
        static f32m4 newInverseVP;

        if (start){
            oldVP = VP;
            start = 0;
        } else{
            oldVP = currMat;
        }
        currMat = VP;
        newInverseVP = glm::inverse(currMat);

        glUniform1i(program->getUniform("renderedTexture"), 0);
        glUniform1i(program->getUniform("depthTexture"), 1);
        glUniform1f(program->getUniform("gamma"), 1.0f / graphicsOptions.gamma);
        glUniform1f(program->getUniform("fExposure"), graphicsOptions.hdrExposure);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, frameBuffer->depthTextureIDs[FB_DRAW]);

        glUniformMatrix4fv(program->getUniform("prevVP"), 1, GL_FALSE, &oldVP[0][0]);
        glUniformMatrix4fv(program->getUniform("inverseVP"), 1, GL_FALSE, &newInverseVP[0][0]);
        glUniform1i(program->getUniform("numSamples"), (int)graphicsOptions.motionBlur);
    } else{
        start = 0;
        program = GameManager::glProgramManager->getProgram("HDR");
        program->use();
        program->enableVertexAttribArrays();
        glUniform1i(program->getUniform("renderedTexture"), 0);
        glUniform1f(program->getUniform("gamma"), 1.0f / graphicsOptions.gamma);
        glUniform1f(program->getUniform("fExposure"), graphicsOptions.hdrExposure);
    }

    const ui32v2 viewport(graphicsOptions.screenWidth, graphicsOptions.screenHeight);
    frameBuffer->draw(viewport, drawMode);

    program->disableVertexAttribArrays();
    program->unuse();
}

