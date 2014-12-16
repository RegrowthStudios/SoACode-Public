#include "stdafx.h"
#include "StarSystemScreen.h"

#include "App.h"
#include "DepthState.h"
#include "GameManager.h"
#include "IOManager.h"
#include "InputDispatcher.h"
#include "InputManager.h"
#include "Inputs.h"
#include "LoadTaskShaders.h"
#include "Options.h"
#include "SkyboxRenderStage.h"
#include "SpaceSystem.h"
#include "SpaceSystemRenderStage.h"

i32 StarSystemScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 StarSystemScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void StarSystemScreen::build() {
    // Empty
}

void StarSystemScreen::destroy(const GameTime& gameTime) {
    // Empty
}

void StarSystemScreen::onEntry(const GameTime& gameTime) {

    mouseButtons[0] = false;
    mouseButtons[0] = false;

    // Init the camera
    m_camera.init(_app->getWindow().getAspectRatio());
    m_camera.setPosition(glm::dvec3(0.0, 200000.0, 0.0));
    m_camera.setDirection(glm::vec3(0.0, -1.0, 0.0));
    m_camera.setUp(glm::cross(m_camera.getRight(), m_camera.getDirection()));
    m_camera.setClippingPlane(10000.0f, 3000000000000.0f);
    m_camera.setTarget(glm::dvec3(0.0, 0.0, 0.0), f32v3(0.0f, -1.0f, 0.0f), f32v3(-1.0f, 0.0f, 0.0f), 200000.0);

    vg::GLProgramManager* glProgramManager = GameManager::glProgramManager;

    _viewport = ui32v4(0, 0, _app->getWindow().getViewportDims());
    

    // Conclass framebuffer
    _hdrFrameBuffer = new vg::GLRenderTarget(_viewport.z, _viewport.w);
    _hdrFrameBuffer->init(vg::TextureInternalFormat::RGBA16F, graphicsOptions.msaa).initDepth();
    if (graphicsOptions.msaa > 0) {
        glEnable(GL_MULTISAMPLE);
    } else {
        glDisable(GL_MULTISAMPLE);
    }

    // Make swap chain
    _swapChain = new vg::RTSwapChain<2>(_viewport.z, _viewport.w);
    _swapChain->init(vg::TextureInternalFormat::RGBA8);
    _quad.init();

    _skyboxRenderStage = new SkyboxRenderStage(glProgramManager->getProgram("Texture"), &m_camera);
    m_spaceSystemRenderStage = new SpaceSystemRenderStage(_app->spaceSystem, &m_camera, glProgramManager->getProgram("BasicColor"), glProgramManager->getProgram("SphericalTerrain"));
    _hdrRenderStage = new HdrRenderStage(glProgramManager, &_quad, &m_camera);


    vui::InputDispatcher::mouse.onMotion.addFunctor(([=](void* s, const vui::MouseMotionEvent& e) { onMouseMotion(s, e); }));
    vui::InputDispatcher::mouse.onButtonDown.addFunctor(([=](void* s, const vui::MouseButtonEvent& e) { onMouseButtonDown(s, e); }));
    vui::InputDispatcher::mouse.onButtonUp.addFunctor(([=](void* s, const vui::MouseButtonEvent& e) { onMouseButtonUp(s, e); }));
    vui::InputDispatcher::mouse.onWheel.addFunctor(([=](void* s, const vui::MouseWheelEvent& e) { onMouseWheel(s, e); }));
    vui::InputDispatcher::key.onKeyDown.addFunctor(([=](void* s, const vui::KeyEvent& e) { onKeyDown(s, e); }));

    GameManager::inputManager->startInput();
}

void StarSystemScreen::onExit(const GameTime& gameTime) {
    GameManager::inputManager->stopInput();
}

void StarSystemScreen::onEvent(const SDL_Event& e) {
    
}

void StarSystemScreen::update(const GameTime& gameTime) {
  
    static double time = 0.0;
    time += 0.001;

    _app->spaceSystem->update(time, m_camera.getPosition());

    // Connect camera to target planet
    float length = m_camera.getFocalLength() / 10.0;
    if (length == 0) length = 0.1;
    m_camera.setClippingPlane(length, m_camera.getFarClip());
    // Target closest point on sphere
    m_camera.setTargetFocalPoint(_app->spaceSystem->getTargetPosition() -
                                f64v3(glm::normalize(m_camera.getDirection())) * _app->spaceSystem->getTargetRadius());

    m_camera.update();

    m_camera.updateProjection();

    GameManager::inputManager->update();

}

void StarSystemScreen::draw(const GameTime& gameTime) {
    // Bind the FBO
    _hdrFrameBuffer->use();
    // Clear depth buffer. Don't have to clear color since skybox will overwrite it
    glClear(GL_DEPTH_BUFFER_BIT);

    // Main render passes
    _skyboxRenderStage->draw();
    DepthState::FULL.set();
    m_spaceSystemRenderStage->draw();

    // Post processing
    _swapChain->reset(0, _hdrFrameBuffer, graphicsOptions.msaa > 0, false);

    // Draw to backbuffer for the last effect
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(_hdrFrameBuffer->getTextureTarget(), _hdrFrameBuffer->getTextureDepthID());
    _hdrRenderStage->draw();

    // Check for errors, just in case
    checkGlError("MainMenuRenderPipeline::render()");
}

void StarSystemScreen::onMouseButtonDown(void* sender, const vui::MouseButtonEvent& e) {
    if (e.button == vui::MouseButton::LEFT) {
        mouseButtons[0] = true;
    } else {
        mouseButtons[1] = true;
    }
}

void StarSystemScreen::onMouseButtonUp(void* sender, const vui::MouseButtonEvent& e) {
    if (e.button == vui::MouseButton::LEFT) {
        mouseButtons[0] = false;
    } else {
        mouseButtons[1] = false;
    }
}

void StarSystemScreen::onMouseWheel(void* sender, const vui::MouseWheelEvent& e) {
#define SCROLL_SPEED 0.1f
    m_camera.offsetTargetFocalLength(m_camera.getTargetFocalLength() * SCROLL_SPEED * -e.dy);
}

void StarSystemScreen::onMouseMotion(void* sender, const vui::MouseMotionEvent& e) {
#define MOUSE_SPEED 0.1f
    if (GameManager::inputManager->getKey(INPUT_MOUSE_LEFT)) {
        m_camera.rotateFromMouse((float)-e.dx, (float)-e.dy, MOUSE_SPEED);
    }
    if (GameManager::inputManager->getKey(INPUT_MOUSE_RIGHT)) {
        m_camera.yawFromMouse((float)e.dx, MOUSE_SPEED);
    }
}

void StarSystemScreen::onKeyDown(void* sender, const vui::KeyEvent& e) {
    switch (e.scanCode) {
        case VKEY_LEFT:
            _app->spaceSystem->offsetTarget(-1);
            break;
        case VKEY_RIGHT:
            _app->spaceSystem->offsetTarget(1);
            break;
    }
}
