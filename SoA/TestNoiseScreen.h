#pragma once

#ifndef TestNoiseScreen_h__
#define TestNoiseScreen_h__

#include <Vorb/ui/IGameScreen.h>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/Timing.h>

struct NoiseType
{
    const char* vertexShader;
    const char* fragmentShader;
};

class TestNoiseScreen : public vui::IGameScreen
{
public:
    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;
    virtual void build() override;
    virtual void destroy(const vui::GameTime& gameTime) override;
    virtual void onEntry(const vui::GameTime& gameTime) override;
    virtual void onExit(const vui::GameTime& gameTime) override;
    virtual void update(const vui::GameTime& gameTime) override;
    virtual void draw(const vui::GameTime& gameTime) override;

    virtual void onNoiseChange();
private:
    vg::GLProgram m_program = nullptr;
    std::vector<NoiseType> m_noiseTypes;
    unsigned int m_currentNoise;
    FpsLimiter m_limiter;
    AutoDelegatePool m_hooks;
};

#endif