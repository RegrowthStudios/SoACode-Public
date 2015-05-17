#include "stdafx.h"
#include "TestVoxelModelScreen.h"


#include <Vorb/colors.h>
#include <Vorb/graphics/GLStates.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/io/IOManager.h>

#include "VoxelModelLoader.h"

#pragma region Simple voxel mesh shader code
const cString SRC_VERT_VOXEL = R"(
uniform mat4 unWVP;

in vec4 vPosition;
in vec3 vColor;

out vec3 fColor;

void main() {
    fColor = vColor;
    gl_Position = unWVP * vPosition;
}
)";
const cString SRC_FRAG_VOXEL = R"(
in vec3 fColor;

out vec4 pColor;

void main() {
    pColor = vec4(fColor, 1);
}
)";
#pragma endregion



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
    m_camera->init(16.0f / 9.0f);
    m_camera->setPosition(f64v3(0, 0, 20));

    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [&](Sender s, const vui::MouseMotionEvent& e) {
        if(m_movingCamera) {
            m_camera->rotateFromMouse(e.dx, e.dy, 1.0f);
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s, const vui::MouseButtonEvent& e) {
        if(e.button == vui::MouseButton::MIDDLE) m_movingCamera = true;
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [&](Sender s, const vui::MouseButtonEvent& e) {
        if(e.button == vui::MouseButton::MIDDLE) m_movingCamera = false;
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
    m_model->loadFromFile("Models\\mammoth.qb");

    vg::GLProgram program(false);
    program.init();
    program.addShader(vg::ShaderType::VERTEX_SHADER, SRC_VERT_VOXEL);
    program.addShader(vg::ShaderType::FRAGMENT_SHADER, SRC_FRAG_VOXEL);
    program.link();
    program.initAttributes();
    program.initUniforms();
    m_model->setShaderProgram(program);

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
    vg::RasterizerState::CULL_CLOCKWISE.set();

    m_model->draw(m_camera->getViewProjectionMatrix());
}