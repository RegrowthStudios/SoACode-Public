#include "stdafx.h"
#include "TestVoxelModelScreen.h"


#include <Vorb/colors.h>
#include <Vorb/graphics/GLStates.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/io/IOManager.h>

#include "VoxelModelLoader.h"
#include "Errors.h"

TestVoxelModelScreen::TestVoxelModelScreen(const App* app) :
IAppScreen<App>(app) {
  // Empty
}

i32 TestVoxelModelScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 TestVoxelModelScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestVoxelModelScreen::build() {
    // Empty
}
void TestVoxelModelScreen::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void TestVoxelModelScreen::onEntry(const vui::GameTime& gameTime) {
    m_camera = new Camera();
    m_camera->init(m_game->getWindow().getAspectRatio());
    m_camera->setPosition(f64v3(0, 0, 20));
    m_camera->setClippingPlane(0.01f, 100000.0f);
    m_camera->setDirection(f32v3(0.0f, 0.0f, -1.0f));

    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [&](Sender s, const vui::MouseMotionEvent& e) {
        if(m_movingCamera) {
            m_camera->rotateFromMouse(e.dx, e.dy, 1.0f);
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s, const vui::MouseButtonEvent& e) {
        if(e.button == vui::MouseButton::LEFT) m_movingCamera = true;
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [&](Sender s, const vui::MouseButtonEvent& e) {
        if(e.button == vui::MouseButton::LEFT) m_movingCamera = false;
    });

    m_movingForward = false;
    m_movingBack = false;
    m_movingLeft = false;
    m_movingRight = false;
    m_movingUp = false;
    m_movingDown = false;

    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s, const vui::KeyEvent& e) {
        switch(e.keyCode) {
        case VKEY_W:
            m_movingForward = true;
            break;
        case VKEY_S:
            m_movingBack = true;
            break;
        case VKEY_A:
            m_movingLeft = true;
            break;
        case VKEY_D:
            m_movingRight = true;
            break;
        case VKEY_SPACE:
            m_movingUp = true;
            break;
        case VKEY_LSHIFT:
        case VKEY_RSHIFT:
            m_movingDown = true;
            break;
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyUp, [&](Sender s, const vui::KeyEvent& e) {
        switch(e.keyCode) {
        case VKEY_W:
            m_movingForward = false;
            break;
        case VKEY_S:
            m_movingBack = false;
            break;
        case VKEY_A:
            m_movingLeft = false;
            break;
        case VKEY_D:
            m_movingRight = false;
            break;
        case VKEY_SPACE:
            m_movingUp = false;
            break;
        case VKEY_LSHIFT:
        case VKEY_RSHIFT:
            m_movingDown = false;
            break;
        }
    });

    m_model = new VoxelModel();
    m_model->loadFromFile("Models/human_female.qb");

    m_renderer.initGL();

    m_movingCamera = false;

    // Set clear state
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);

}
void TestVoxelModelScreen::onExit(const vui::GameTime& gameTime) {
    // Empty
}

void TestVoxelModelScreen::update(const vui::GameTime& gameTime) {
    f32 speed = 5.0f;
    if(m_movingForward) {
        f32v3 offset = m_camera->getDirection() * speed * (f32)gameTime.elapsed;
        m_camera->offsetPosition(offset);
    }
    if(m_movingBack) {
        f32v3 offset = m_camera->getDirection() * -speed * (f32)gameTime.elapsed;
        m_camera->offsetPosition(offset);
    }
    if(m_movingLeft) {
        f32v3 offset = m_camera->getRight() * speed * (f32)gameTime.elapsed;
        m_camera->offsetPosition(offset);
    }
    if(m_movingRight) {
        f32v3 offset = m_camera->getRight() * -speed * (f32)gameTime.elapsed;
        m_camera->offsetPosition(offset);
    }
    if(m_movingUp) {
        f32v3 offset = f32v3(0, 1, 0) * speed * (f32)gameTime.elapsed;
        m_camera->offsetPosition(offset);
    }
    if(m_movingDown) {
        f32v3 offset = f32v3(0, 1, 0) *  -speed * (f32)gameTime.elapsed;
        m_camera->offsetPosition(offset);
    }
    m_camera->update();
}
void TestVoxelModelScreen::draw(const vui::GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vg::DepthState::FULL.set();
    vg::RasterizerState::CULL_NONE.set();

    m_renderer.draw(m_model, m_camera->getViewProjectionMatrix(), -f32v3(m_camera->getPosition()));
    checkGlError("TestVoxelModelScreen::draw");
}