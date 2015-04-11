#include "stdafx.h"
#include "TestStarScreen.h"

#include "soaUtils.h"
#include "Errors.h"
#include <Vorb\graphics\GLProgram.h>
#include <Vorb\graphics\GpuMemory.h>
#include <Vorb\graphics\ImageIO.h>
#include <Vorb\io\IOManager.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/Timing.h>

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

i32 TestStarScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestStarScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestStarScreen::build() {

}
void TestStarScreen::destroy(const vui::GameTime& gameTime) {

}
static float eyePos = 1.5f;
void TestStarScreen::onEntry(const vui::GameTime& gameTime) {

    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s, const vui::KeyEvent& e) {
        if (e.keyCode == VKEY_F1) {
            m_starRenderer.disposeShaders();
        } else if (e.keyCode == VKEY_UP) {
            m_isUpDown = true;
        } else if (e.keyCode == VKEY_DOWN) {
            m_isDownDown = true;
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyUp, [&](Sender s, const vui::KeyEvent& e) {
        if (e.keyCode == VKEY_UP) {
            m_isUpDown = false;
        } else if (e.keyCode == VKEY_DOWN) {
            m_isDownDown = false;
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onWheel, [&](Sender s, const vui::MouseWheelEvent& e) {
        eyePos += -e.dy * 0.025f * glm::length(eyePos);
    });
    glEnable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0);

    m_eyePos = f32v3(0, 0, eyePos);

    // Set up components
    m_sCmp.radius = 1.0;
    m_sCmp.temperature = 5853.0;
}

void TestStarScreen::onExit(const vui::GameTime& gameTime) {

}

void TestStarScreen::update(const vui::GameTime& gameTime) {
    m_eyePos = f32v3(0, 0, eyePos);

    const float TMP_INC = 10.0;

    if (m_isDownDown) {
        m_sCmp.temperature -= TMP_INC;
    } else if (m_isUpDown) {
        m_sCmp.temperature += TMP_INC;
    }
    printf("Temp: %lf\n", m_sCmp.temperature);
}

void TestStarScreen::draw(const vui::GameTime& gameTime) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PreciseTimer timer;
    timer.start();
    f32m4 V = glm::lookAt(m_eyePos, f32v3(0.0f), f32v3(0.0f, 1.0f, 0.0f));
    m_starRenderer.draw(m_sCmp, glm::perspectiveFov(90.0f, 1280.0f, 720.0f, 0.1f, 1000.0f) * V, V,
                            f64q(), m_eyePos);
    glFinish();
  //  std::cout << timer.stop() << std::endl;
    checkGlError("TestStarScreen::draw");
}

