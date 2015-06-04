#pragma once

#ifndef TestNoiseScreen_h__
#define TestNoiseScreen_h__

#include <Vorb/ui/IGameScreen.h>
#include <Vorb/graphics/GLProgram.h>

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
private:
    vg::GLProgram* m_program;
};

#endif