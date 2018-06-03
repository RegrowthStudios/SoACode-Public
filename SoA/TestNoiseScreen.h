#pragma once

#ifndef TestNoiseScreen_h__
#define TestNoiseScreen_h__

#include <Vorb/ui/IGameScreen.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/Timing.h>
#include <Vorb/graphics/SpriteBatch.h>

#include "App.h"

const int NUM_TEST_NOISE_TYPES = 2;
enum TEST_NOISE_TYPES { SIMPLEX = 0, CELLULAR };

class TestNoiseScreen : public vui::IAppScreen <App>
{
public:
    TestNoiseScreen(const App* app);
    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;
    virtual void build() override;
    virtual void destroy(const vui::GameTime& gameTime) override;
    virtual void onEntry(const vui::GameTime& gameTime) override;
    virtual void onExit(const vui::GameTime& gameTime) override;
    virtual void update(const vui::GameTime& gameTime) override;
    virtual void draw(const vui::GameTime& gameTime) override;
private:
    void TestNoiseScreen::onNoiseChange();
    vg::SpriteBatch m_sb;
    vg::SpriteFont m_font;
    TEST_NOISE_TYPES m_currentNoise = SIMPLEX;
    FpsLimiter m_limiter;
    AutoDelegatePool m_hooks;
    VGTexture m_textures[NUM_TEST_NOISE_TYPES];
    f64 m_times[NUM_TEST_NOISE_TYPES];
};

#endif