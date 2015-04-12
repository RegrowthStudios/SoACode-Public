#include "stdafx.h"
#include "TestStarScreen.h"

#include "soaUtils.h"
#include "Errors.h"
#include "HdrRenderStage.h"
#include <Vorb/Timing.h>
#include <Vorb/colors.h>
#include <Vorb/graphics/DepthState.h>
#include <Vorb/graphics/RasterizerState.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/GpuMemory.h>
#include <Vorb/graphics/ImageIO.h>
#include <Vorb/io/IOManager.h>

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

const f64 STAR_RADIUS = 1693000.0 / 2.0;

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
static f32 eyePos = (f32)STAR_RADIUS;
void TestStarScreen::onEntry(const vui::GameTime& gameTime) {

    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s, const vui::KeyEvent& e) {
        if (e.keyCode == VKEY_F1) {
            m_starRenderer.disposeShaders();
            m_hdr->reloadShader();
        } else if (e.keyCode == VKEY_UP) {
            m_isUpDown = true;
        } else if (e.keyCode == VKEY_DOWN) {
            m_isDownDown = true;
        } else if (e.keyCode == VKEY_H) {
            m_isHDR = !m_isHDR;
        } else if (e.keyCode == VKEY_G) {
            m_isGlow = !m_isGlow;
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

    m_eyePos = f32v3(0, 0, STAR_RADIUS + 100.0 + eyePos);

    // Set up components
    m_sCmp.radius = STAR_RADIUS;
    m_sCmp.temperature = 3525.0;

    m_spriteBatch.init();
    m_spriteFont.init("Fonts/orbitron_black-webfont.ttf", 32);

    m_hdrFrameBuffer = new vg::GLRenderTarget(_game->getWindow().getViewportDims());
    m_hdrFrameBuffer->init(vg::TextureInternalFormat::RGBA16F, 0).initDepth();

    m_quad.init();
    m_hdr = new HdrRenderStage(&m_quad, &m_camera);
}

void TestStarScreen::onExit(const vui::GameTime& gameTime) {
    delete m_hdr;
    delete m_hdrFrameBuffer;
}

void TestStarScreen::update(const vui::GameTime& gameTime) {
    m_eyePos = f32v3(0, 0, STAR_RADIUS + 100.0 + eyePos);

    const float TMP_INC = 25.0;

    if (m_isDownDown) {
        m_sCmp.temperature -= TMP_INC;
    } else if (m_isUpDown) {
        m_sCmp.temperature += TMP_INC;
    }
}

void TestStarScreen::draw(const vui::GameTime& gameTime) {

    if (m_isHDR) m_hdrFrameBuffer->use();

    f32 width = _game->getWindow().getWidth();
    f32 height = _game->getWindow().getHeight();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    PreciseTimer timer;
    timer.start();

    m_camera.setAspectRatio(width / height);
    m_camera.setClippingPlane(eyePos, (f32)(STAR_RADIUS * 100.0));
    m_camera.setFieldOfView(90.0f);
    m_camera.setPosition(f64v3(m_eyePos));
    m_camera.setDirection(f32v3(0.0f, 0.0f, -1.0f));
    m_camera.setUp(f32v3(0.0f, 1.0f, 0.0f));
    m_camera.update();

    // Render the star
    m_starRenderer.drawStar(m_sCmp, m_camera.getViewProjectionMatrix(), f64q(), m_eyePos);
    m_starRenderer.drawCorona(m_sCmp, m_camera.getViewProjectionMatrix(), m_camera.getViewMatrix(), m_eyePos);
    if (m_isGlow) m_starRenderer.drawGlow(m_sCmp, m_camera.getViewProjectionMatrix(), m_camera.getViewMatrix(), m_eyePos);

    if (m_isHDR) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(m_hdrFrameBuffer->getTextureTarget(), m_hdrFrameBuffer->getTextureID());
        m_hdr->render();
    }

    glFinish();
  //  std::cout << timer.stop() << std::endl;
    checkGlError("TestStarScreen::draw");

    // Draw the temperature and HDR
    char buf[256];
    sprintf(buf, "Temperature (K): %.0lf", m_sCmp.temperature);
    m_spriteBatch.begin();
    m_spriteBatch.drawString(&m_spriteFont, buf, f32v2(30.0f, 30.0f), f32v2(1.0f), color::AliceBlue);
    if (m_isHDR) {
        m_spriteBatch.drawString(&m_spriteFont, "HDR: Enabled", f32v2(30.0f, 65.0f), f32v2(1.0f), color::AliceBlue);
    } else {
        m_spriteBatch.drawString(&m_spriteFont, "HDR: Disabled", f32v2(30.0f, 65.0f), f32v2(1.0f), color::AliceBlue);
    }
    if (m_isGlow) {
        m_spriteBatch.drawString(&m_spriteFont, "Glow: Enabled", f32v2(30.0f, 100.0f), f32v2(1.0f), color::AliceBlue);
    } else {
        m_spriteBatch.drawString(&m_spriteFont, "Glow: Disabled", f32v2(30.0f, 100.0f), f32v2(1.0f), color::AliceBlue);
    }
    
    m_spriteBatch.end();
    m_spriteBatch.renderBatch(f32v2(width, height));
    vg::DepthState::FULL.set();
    vg::RasterizerState::CULL_CLOCKWISE.set();
}

