#include "stdafx.h"
#include "StarSystemScreen.h"

#include <Vorb/DepthState.h>
#include <Vorb/IOManager.h>
#include <Vorb/InputDispatcher.h>
#include <Vorb/Timing.h>

#include "App.h"

#include "GameManager.h"
#include "InputManager.h"
#include "Inputs.h"
#include "LoadTaskShaders.h"
#include "Options.h"
#include "SkyboxRenderStage.h"
#include "SpaceSystem.h"
#include "SpaceSystemRenderStage.h"
#include "MeshManager.h"

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

// Permutation table.  The same list is repeated twice.
static const ui8 perm[512] = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
    8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
    35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
    134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
    55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
    18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226,
    250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
    189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43,
    172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97,
    228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
    107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
    138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,

    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
    8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
    35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
    134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
    55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
    18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226,
    250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
    189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43,
    172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97,
    228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
    107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
    138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

static const ui8 grad3[12][3] = {
    { 2, 2, 1 }, { 0, 2, 1 }, { 2, 0, 1 }, { 0, 0, 1 },
    { 2, 1, 2 }, { 0, 1, 2 }, { 2, 1, 0 }, { 0, 1, 0 },
    { 1, 2, 2 }, { 1, 0, 2 }, { 1, 2, 0 }, { 1, 0, 0 }
};

void StarSystemScreen::onEntry(const GameTime& gameTime) {

    m_inputManager = new InputManager;

    mouseButtons[0] = false;
    mouseButtons[1] = false;

    _app->spaceSystem->targetBody("Aldrin");

    // Init the camera
    m_camera.init(_app->getWindow().getAspectRatio());
    m_camera.setPosition(glm::dvec3(0.0, 200000.0, 0.0));
    m_camera.setDirection(glm::vec3(0.0, -1.0, 0.0));
    m_camera.setRight(f32v3(1.0, 0.0, 0.0));
    m_camera.setUp(glm::cross(m_camera.getRight(), m_camera.getDirection()));
    m_camera.setClippingPlane(1000.0f, 3000000000000.0f);
    m_camera.setTarget(glm::dvec3(0.0, 0.0, 0.0), f32v3(0.0f, -1.0f, 0.0f), f32v3(1.0f, 0.0f, 0.0f), 6000.0);

    vg::GLProgramManager* glProgramManager = GameManager::glProgramManager;

    _viewport = ui32v4(0, 0, _app->getWindow().getViewportDims());
    

    // Construct framebuffer
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
    m_spaceSystemRenderStage = new SpaceSystemRenderStage(ui32v2(_viewport.z, _viewport.w),
                                                          _app->spaceSystem, &m_camera,
                                                          glProgramManager->getProgram("BasicColor"),
                                                          glProgramManager->getProgram("SphericalTerrain"),
                                                          glProgramManager->getProgram("SphericalWater"),
                                                          GameManager::textureCache->addTexture("Textures/selector.png").id);
    _hdrRenderStage = new HdrRenderStage(glProgramManager, &_quad, &m_camera);

    vui::InputDispatcher::mouse.onMotion.addFunctor(([=](void* s, const vui::MouseMotionEvent& e) { onMouseMotion(s, e); }));
    vui::InputDispatcher::mouse.onButtonDown.addFunctor(([=](void* s, const vui::MouseButtonEvent& e) { onMouseButtonDown(s, e); }));
    vui::InputDispatcher::mouse.onButtonUp.addFunctor(([=](void* s, const vui::MouseButtonEvent& e) { onMouseButtonUp(s, e); }));
    vui::InputDispatcher::mouse.onWheel.addFunctor(([=](void* s, const vui::MouseWheelEvent& e) { onMouseWheel(s, e); }));
    vui::InputDispatcher::key.onKeyDown.addFunctor(([=](void* s, const vui::KeyEvent& e) { onKeyDown(s, e); }));
}

void StarSystemScreen::onExit(const GameTime& gameTime) {
    m_inputManager->stopInput();
    delete m_inputManager;
}

void StarSystemScreen::onEvent(const SDL_Event& e) {
    
}

void StarSystemScreen::update(const GameTime& gameTime) {
  
    static double time = 0.0;
    time += 0.0000001;

    _app->spaceSystem->update(time, m_camera.getPosition());
    _app->spaceSystem->glUpdate();

    // Connect camera to target planet
    float length = m_camera.getFocalLength() / 10.0;
    if (length == 0) length = 0.1;
    m_camera.setClippingPlane(length, m_camera.getFarClip());
    // Target closest point on sphere
    m_camera.setTargetFocalPoint(_app->spaceSystem->getTargetPosition() -
                                f64v3(glm::normalize(m_camera.getDirection())) * _app->spaceSystem->getTargetRadius());

    m_camera.update();

    m_camera.updateProjection();

    m_inputManager->update();

}

void StarSystemScreen::draw(const GameTime& gameTime) {
    PreciseTimer timer;
 //   timer.start();
    // Bind the FBO
    _hdrFrameBuffer->use();
    // Clear depth buffer. Don't have to clear color since skybox will overwrite it
    glClear(GL_DEPTH_BUFFER_BIT);

    // Main render passes
    _skyboxRenderStage->draw();
    DepthState::FULL.set();
    m_spaceSystemRenderStage->draw();

    DepthState::NONE.set();

    vg::GLProgram* noiseProg = GameManager::glProgramManager->getProgram("SimplexNoise");
    noiseProg->use();
    noiseProg->enableVertexAttribArrays();

  //  _quad.draw();
    noiseProg->disableVertexAttribArrays();
    noiseProg->unuse();

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
    glFlush();
    glFinish();
  //  std::cout << timer.stop() << std::endl;
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
    if (mouseButtons[0]) {
        m_camera.rotateFromMouse((float)-e.dx, (float)-e.dy, MOUSE_SPEED);
    }
    if (mouseButtons[1]) {
        m_camera.yawFromMouse((float)e.dx, MOUSE_SPEED);
    }
}

void StarSystemScreen::onKeyDown(void* sender, const vui::KeyEvent& e) {
    switch (e.keyCode) {
        case VKEY_LEFT:
            _app->spaceSystem->offsetTarget(-1);
            break;
        case VKEY_RIGHT:
            _app->spaceSystem->offsetTarget(1);
            break;
        case VKEY_F10: // Reload star system
            delete _app->spaceSystem;
            const_cast<App*>(_app)->spaceSystem = new SpaceSystem();
            const_cast<App*>(_app)->spaceSystem->init(GameManager::glProgramManager);
            const_cast<App*>(_app)->spaceSystem->addSolarSystem("StarSystems/Trinity");
            _hdrFrameBuffer = new vg::GLRenderTarget(_viewport.z, _viewport.w);
            _hdrFrameBuffer->init(vg::TextureInternalFormat::RGBA16F, graphicsOptions.msaa).initDepth();
            if (graphicsOptions.msaa > 0) {
                glEnable(GL_MULTISAMPLE);
            } else {
                glDisable(GL_MULTISAMPLE);
            }
            _app->spaceSystem->targetBody("Aldrin");
            delete m_spaceSystemRenderStage;
            m_spaceSystemRenderStage = new SpaceSystemRenderStage(_app->getWindow().getViewportDims(),
                                                                  _app->spaceSystem, &m_camera,
                                                                  GameManager::glProgramManager->getProgram("BasicColor"),
                                                                  GameManager::glProgramManager->getProgram("SphericalTerrain"),
                                                                  GameManager::glProgramManager->getProgram("SphericalWater"),
                                                                  GameManager::textureCache->addTexture("Textures/selector.png").id);
            break;
        case VKEY_F11: // Reload everything
            GameManager::glProgramManager->destroy();
            { // Load shaders
                vcore::RPCManager glrpc;
                LoadTaskShaders shaderTask(&glrpc);
                std::thread([&] () {
                    shaderTask.doWork();
                }).detach();
                while (!shaderTask.isFinished()) {
                    if (glrpc.processRequests() == 0) Sleep(50);
                }
            }

            delete _app->spaceSystem;
            const_cast<App*>(_app)->spaceSystem = new SpaceSystem();
            const_cast<App*>(_app)->spaceSystem->init(GameManager::glProgramManager);
            const_cast<App*>(_app)->spaceSystem->addSolarSystem("StarSystems/Trinity");
            _hdrFrameBuffer = new vg::GLRenderTarget(_viewport.z, _viewport.w);
            _hdrFrameBuffer->init(vg::TextureInternalFormat::RGBA16F, graphicsOptions.msaa).initDepth();
            if (graphicsOptions.msaa > 0) {
                glEnable(GL_MULTISAMPLE);
            } else {
                glDisable(GL_MULTISAMPLE);
            }
            _app->spaceSystem->targetBody("Aldrin");
            // Make swap chain
            _swapChain = new vg::RTSwapChain<2>(_viewport.z, _viewport.w);
            _swapChain->init(vg::TextureInternalFormat::RGBA8);
            _quad.init();

            delete _skyboxRenderStage;
            delete m_spaceSystemRenderStage;
            delete _hdrRenderStage;

            _skyboxRenderStage = new SkyboxRenderStage(GameManager::glProgramManager->getProgram("Texture"), &m_camera);
            m_spaceSystemRenderStage = new SpaceSystemRenderStage(_app->getWindow().getViewportDims(),
                                                                  _app->spaceSystem, &m_camera,
                                                                  GameManager::glProgramManager->getProgram("BasicColor"),
                                                                  GameManager::glProgramManager->getProgram("SphericalTerrain"),
                                                                  GameManager::glProgramManager->getProgram("SphericalWater"),
                                                                  GameManager::textureCache->addTexture("Textures/selector.png").id);
            _hdrRenderStage = new HdrRenderStage(GameManager::glProgramManager, &_quad, &m_camera);
            break;
    }
}