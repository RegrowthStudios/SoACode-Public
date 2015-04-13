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

void TestGasGiantScreen::onEntry(const vui::GameTime& gameTime) {
 
    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s, const vui::KeyEvent& e) {
        if(e.keyCode == VKEY_F1) {
            m_gasGiantRenderer.disposeShader();
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onWheel, [&](Sender s, const vui::MouseWheelEvent& e) {
        m_eyeDist += -e.dy * 0.025 * glm::length(m_eyeDist);
    });
    glEnable(GL_DEPTH_TEST);

    vg::BitmapResource rs = vg::ImageIO().load("Textures/Test/GasGiantLookup2.png");
    if (rs.data == nullptr) pError("Failed to load gas giant texture");
    VGTexture colorBandLookup = vg::GpuMemory::uploadTexture(rs.bytesUI8, rs.width, rs.height,
                                                             &vg::SamplerState::LINEAR_CLAMP);

    vg::ImageIO().free(rs);

    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0);

    m_eyePos = f32v3(0, 0, m_eyeDist);

    // Set up components
    m_ggCmp.radius = GIANT_RADIUS;
    m_ggCmp.colorMap = colorBandLookup;
    m_aCmp.radius = GIANT_RADIUS * 1.025;
    m_aCmp.planetRadius = GIANT_RADIUS;
    m_aCmp.invWavelength4 = f32v3(1.0f / powf(0.475f, 4.0f),
                                  1.0f / powf(0.57f, 4.0f),
                                  1.0f / powf(0.65f, 4.0f));

    m_camera.setFieldOfView(90.0f);
    f32 width = _game->getWindow().getWidth();
    f32 height = _game->getWindow().getHeight();
    m_camera.setAspectRatio(width / height);
    m_camera.setDirection(f32v3(0.0f, 0.0f, -1.0f));
    m_camera.setUp(f32v3(0.0f, 1.0f, 0.0f));
}

void TestGasGiantScreen::onExit(const vui::GameTime& gameTime) {

}

void TestGasGiantScreen::update(const vui::GameTime& gameTime) {
    m_eyePos = f64v3(0, 0, GIANT_RADIUS + m_eyeDist + 100.0);
}

void TestGasGiantScreen::draw(const vui::GameTime& gameTime) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_camera.setClippingPlane((f32)(m_eyeDist / 2.0), (f32)(m_eyeDist + GIANT_RADIUS * 10.0));
    m_camera.setPosition(f64v3(m_eyePos));
    m_camera.update();

    f32v3 lightPos = glm::normalize(f32v3(0.0f, 0.0f, 1.0f));

    PreciseTimer timer;
    m_gasGiantRenderer.draw(m_ggCmp, m_camera.getViewProjectionMatrix(),
                            f64q(), f32v3(m_eyePos), lightPos, &m_slCmp, &m_aCmp);

    m_atmoRenderer.draw(m_aCmp, m_camera.getViewProjectionMatrix(), f32v3(m_eyePos), lightPos, &m_slCmp);
    //glFinish();
   
    checkGlError("TestGasGiantScreen::draw");
}
