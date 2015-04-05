#include "stdafx.h"
#include "TestGasGiantScreen.h"
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

i32 TestGasGiantScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestGasGiantScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestGasGiantScreen::build() {

}
void TestGasGiantScreen::destroy(const vui::GameTime& gameTime) {

}
static float eyePos = 1.5f;
void TestGasGiantScreen::onEntry(const vui::GameTime& gameTime) {
 
    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s, const vui::KeyEvent& e) {
        if(e.keyCode == VKEY_F1) {
            m_gasGiantRenderer.disposeShader();
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onWheel, [&](Sender s, const vui::MouseWheelEvent& e) {
        eyePos += -e.dy * 0.01f;
    });
    glEnable(GL_DEPTH_TEST);

    vg::BitmapResource rs = vg::ImageIO().load("Textures/Test/GasGiantLookup.png");
    if (rs.data == nullptr) pError("Failed to load gas giant texture");
    VGTexture colorBandLookup = vg::GpuMemory::uploadTexture(rs.bytesUI8, rs.width, rs.height,
                                                             &vg::SamplerState::LINEAR_CLAMP);

    vg::ImageIO().free(rs);

    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0);

    m_eyePos = f32v3(0, 0, eyePos);

    // Set up components
    m_ggCmp.radius = 1.0;
    m_ggCmp.colorMap = colorBandLookup;
}

void TestGasGiantScreen::onExit(const vui::GameTime& gameTime) {

}

void TestGasGiantScreen::update(const vui::GameTime& gameTime) {
    static float dt = 1.0f;
    dt += 0.001f;
    f32v3 eulerAngles(0, dt, 0);
    f32q rotationQuat = f32q(eulerAngles);
    m_eyePos = f32v3(0, 0, eyePos) * rotationQuat;
}

void TestGasGiantScreen::draw(const vui::GameTime& gameTime) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    PreciseTimer timer;
    timer.start();
    m_gasGiantRenderer.draw(m_ggCmp, glm::perspectiveFov(90.0f, 1280.0f, 720.0f, 0.1f, 1000.0f) * glm::lookAt(f32v3(0.0f), -m_eyePos, f32v3(0.0f, 1.0f, 0.0f)),
                            m_eyePos, f32v3(-1.0f, 0.0f, 0.0f), &m_slCmp);
    glFinish();
    std::cout << timer.stop() << std::endl;
    checkGlError("TestGasGiantScreen::draw");
}


//pColor = vec4(texture(unColorBandLookup, vec2(0.5, fPosition.y)).rgb, 1.0);