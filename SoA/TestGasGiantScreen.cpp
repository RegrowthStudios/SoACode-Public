#include "stdafx.h"
#include "TestGasGiantScreen.h"
#include "soaUtils.h"
#include "GasGiantRenderer.h"
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
            m_gasGiantRenderer->dispose();
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onWheel, [&](Sender s, const vui::MouseWheelEvent& e) {
        eyePos += -e.dy * 0.01f;
    });
    glEnable(GL_DEPTH_TEST);
    
    m_gasGiantRenderer = new GasGiantRenderer();
    //vg::Texture tex = m_textureCache.addTexture("Textures/Test/GasGiantLookup.png", &SamplerState::POINT_CLAMP);
    //m_gasGiantRenderer->setColorBandLookupTexture(tex.id);

    VGTexture colorBandLookup;
    glGenTextures(1, &colorBandLookup);
    glBindTexture(GL_TEXTURE_2D, colorBandLookup);
    
    vg::BitmapResource rs = vg::ImageIO().load("Textures/Test/GasGiantLookup.png");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, rs.width, rs.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rs.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    m_gasGiantRenderer->setColorBandLookupTexture(colorBandLookup);

    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0);
    m_eyePos = f32v3(0, 0, eyePos);
}

void TestGasGiantScreen::onExit(const vui::GameTime& gameTime) {
    delete m_gasGiantRenderer;
}
static float dt = 1.0f;
void TestGasGiantScreen::update(const vui::GameTime& gameTime) {
    static float dt = 1.0f;
    dt += 0.001f;
    f32v3 eulerAngles(0, dt, 0);
    f32q rotationQuat = f32q(eulerAngles);
    m_eyePos = f32v3(0, 0, eyePos) * rotationQuat;
}

void TestGasGiantScreen::draw(const vui::GameTime& gameTime) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    f32m4 mvp = glm::translate(f32m4(1.0f), f32v3(0, 0, 0)) * glm::perspectiveFov(90.0f, 1280.0f, 720.0f, 0.1f, 1000.0f) * glm::lookAt(m_eyePos, f32v3(0, 0, 0), f32v3(0, 1, 0));
    
    PreciseTimer timer;
    timer.start();
    m_gasGiantRenderer->render(mvp);
    glFinish();
    std::cout << timer.stop() << std::endl;
}


//pColor = vec4(texture(unColorBandLookup, vec2(0.5, fPosition.y)).rgb, 1.0);