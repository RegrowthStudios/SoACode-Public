#include "stdafx.h"
#include "TestNoiseScreen.h"

#include <Vorb/ui/InputDispatcher.h>
#include <SDL2/SDL.h>
#include <Vorb/colors.h>
#include <Vorb/graphics/GpuMemory.h>

#include "Errors.h"
#include "ShaderLoader.h"
#include "Noise.h"
#include "soaUtils.h"

#define MS_AVARAGE_FRAMES 60

auto startTime = std::chrono::high_resolution_clock::now();

f64 ms = 0;
unsigned int frameCount = 0;

TestNoiseScreen::TestNoiseScreen(const App* app) :
IAppScreen<App>(app) {

}

i32 TestNoiseScreen::getNextScreen() const
{
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestNoiseScreen::getPreviousScreen() const
{
    return SCREEN_INDEX_NO_SCREEN;
}

void TestNoiseScreen::build()
{

}
void TestNoiseScreen::destroy(const vui::GameTime& gameTime VORB_UNUSED)
{

}

void TestNoiseScreen::onEntry(const vui::GameTime& gameTime VORB_MAYBE_UNUSED)
{ 
    m_sb.init();
    m_font.init("Fonts/orbitron_bold-webfont.ttf", 32);

    for (int i = 0; i < NUM_TEST_NOISE_TYPES; i++) {
        m_textures[i] = 0;
    }

    onNoiseChange();

    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s VORB_MAYBE_UNUSED, const vui::KeyEvent& e) {
        if (e.keyCode == VKEY_LEFT) {
            if (m_currentNoise == 0) {
                m_currentNoise = TEST_NOISE_TYPES::CELLULAR;
            } else {
                m_currentNoise = (TEST_NOISE_TYPES)((int)m_currentNoise - 1);
            }
        }
        if (e.keyCode == VKEY_RIGHT) {
            if (m_currentNoise == TEST_NOISE_TYPES::CELLULAR) {
                m_currentNoise = TEST_NOISE_TYPES::SIMPLEX;
            } else {
                m_currentNoise = (TEST_NOISE_TYPES)((int)m_currentNoise + 1);
            }
        }
        onNoiseChange();
    });
}

void TestNoiseScreen::onExit(const vui::GameTime& gameTime VORB_UNUSED)
{
}

void TestNoiseScreen::update(const vui::GameTime& gameTime VORB_UNUSED)
{
}

void TestNoiseScreen::draw(const vui::GameTime& gameTime VORB_MAYBE_UNUSED)
{
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int numSamples = m_app->getWindow().getWidth() * (m_app->getWindow().getHeight() - 100);

    f32v2 destDims = f32v2(m_app->getWindow().getViewportDims());
    destDims.y += 100.0f;

    // Texture
    m_sb.begin();
    m_sb.draw(m_textures[m_currentNoise], f32v2(0.0f, 100.0f), destDims, color::White);
    m_sb.end();
    m_sb.render(f32v2(m_app->getWindow().getViewportDims()));

    // UI
    m_sb.begin();
    switch (m_currentNoise) {
        case SIMPLEX:
            m_sb.drawString(&m_font, "Simplex", f32v2(30.0f), f32v2(0.7f), color::White);
            break;
        case CELLULAR:
            m_sb.drawString(&m_font, "Cellular", f32v2(30.0f), f32v2(0.7f), color::White);
            break;
    }
    char buf[256];
    sprintf(buf, "Time %.2lf ms", m_times[m_currentNoise]);
    m_sb.drawString(&m_font, buf, f32v2(30.0f, 60.0f), f32v2(0.7f), color::White);

    sprintf(buf, "Samples %d", numSamples);
    m_sb.drawString(&m_font, buf, f32v2(330.0f, 60.0f), f32v2(0.7f), color::White);

    sprintf(buf, "Time per sample: %.6lf ms", m_times[m_currentNoise] / numSamples);
    m_sb.drawString(&m_font, buf, f32v2(630.0f, 60.0f), f32v2(0.7f), color::White);

    m_sb.end();
    m_sb.render(f32v2(m_app->getWindow().getViewportDims()));

}

void TestNoiseScreen::onNoiseChange()
{
    // Only generate once
    if (m_textures[m_currentNoise]) return;
    const f64 frequency = 0.01;
    int width = m_app->getWindow().getWidth();
    int height = m_app->getWindow().getHeight() - 100;
    std::vector<color4> buffer;
    buffer.resize(width * height);
    PreciseTimer timer;
    timer.start();
    f64 val;
    ui8 p;
    f64v2 cval;
    switch (m_currentNoise) {
        case SIMPLEX:
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    val = Noise::raw((f64)x * frequency, (f64)y * frequency, 0.0);
                    p = (ui8)((val + 1.0) * 127.5);
                    buffer[y * width + x].r = p;
                    buffer[y * width + x].g = p;
                    buffer[y * width + x].b = p;
                    buffer[y * width + x].a = 255;
                }
            }
            break;
        case CELLULAR:
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    cval = Noise::cellular(f64v3((f64)x * frequency, (f64)y * frequency, 0.0));
                    val = (cval.y - cval.x);
                    p = (ui8)((val + 1.0) * 127.5);
                    buffer[y * width + x].r = p;
                    buffer[y * width + x].g = p;
                    buffer[y * width + x].b = p;
                    buffer[y * width + x].a = 255;
                }
            }
            break;
    }
    m_times[m_currentNoise] = timer.stop();
    m_textures[m_currentNoise] = vg::GpuMemory::uploadTexture((void*)buffer.data(), width, height);
}