#include "stdafx.h"
#include "TestVoxelModelScreen.h"


#include <Vorb/colors.h>
#include <Vorb/graphics/GLStates.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/io/IOManager.h>

#include "DebugRenderer.h"
#include "Errors.h"
#include "ModelMesher.h"
#include "VoxelModelLoader.h"
#include "soaUtils.h"

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
  
    m_camera.init(m_game->getWindow().getAspectRatio());
    m_camera.setPosition(f64v3(0, 0, 0));
    m_camera.setClippingPlane(0.01f, 100000.0f);
    m_camera.setDirection(f32v3(0.0f, 0.0f, -1.0f));
    m_camera.setRight(f32v3(-1.0f, 0.0f, 0.0f));
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [&](Sender s, const vui::MouseMotionEvent& e) {
        if (m_mouseButtons[0]) {
            m_camera.rotateFromMouse(-e.dx, -e.dy, 0.1f);
        }
        if (m_mouseButtons[1]) {
            m_camera.rollFromMouse((f32)e.dx, 0.1f);
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s, const vui::MouseButtonEvent& e) {
        if (e.button == vui::MouseButton::LEFT) m_mouseButtons[0] = true;
        if (e.button == vui::MouseButton::RIGHT) m_mouseButtons[1] = true;
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [&](Sender s, const vui::MouseButtonEvent& e) {
        if (e.button == vui::MouseButton::LEFT) m_mouseButtons[0] = false;
        if (e.button == vui::MouseButton::RIGHT) m_mouseButtons[1] = false;
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
            m_movingFast = true;
            break;
        case VKEY_M:
            m_wireFrame = !m_wireFrame;
            break;
        case VKEY_N:
            m_currentMesh++;
            if (m_currentMesh > 1) m_currentMesh = 0;
            break;
        case VKEY_F1:
            // Reload shader
            m_renderer.dispose();
            m_renderer.initGL();
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
            m_movingFast = false;
            break;
        }
    });

    m_model = new VoxelModel();
    m_model->loadFromFile("Models/human_female.qb");

    m_currentMesh = 0;
    m_meshes[0] = ModelMesher::createMesh(m_model);
    m_meshes[1] = ModelMesher::createMarchingCubesMesh(m_model);

    m_renderer.initGL();

    m_mouseButtons[0] = false;
    m_mouseButtons[1] = false;
    m_mouseButtons[2] = false;

    // Set clear state
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);

}
void TestVoxelModelScreen::onExit(const vui::GameTime& gameTime) {
    // Empty
}

void TestVoxelModelScreen::update(const vui::GameTime& gameTime) {
    f32 speed = 5.0f;
    if (m_movingFast) speed *= 5.0f;
    if(m_movingForward) {
        f32v3 offset = m_camera.getDirection() * speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    if(m_movingBack) {
        f32v3 offset = m_camera.getDirection() * -speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    if(m_movingLeft) {
        f32v3 offset = m_camera.getRight() * -speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    if(m_movingRight) {
        f32v3 offset = m_camera.getRight() * speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    if(m_movingUp) {
        f32v3 offset = f32v3(0, 1, 0) * speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    if(m_movingDown) {
        f32v3 offset = f32v3(0, 1, 0) *  -speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    m_camera.update();
}
void TestVoxelModelScreen::draw(const vui::GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vg::DepthState::FULL.set();
    vg::RasterizerState::CULL_CLOCKWISE.set();

    if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    m_model->setMesh(m_meshes[m_currentMesh]);
    m_renderer.draw(m_model, m_camera.getViewProjectionMatrix(), m_camera.getPosition(), f64q());
    if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    checkGlError("TestVoxelModelScreen::draw");
}