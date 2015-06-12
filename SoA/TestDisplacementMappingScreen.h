#pragma once

#ifndef TestDisplacementMappingScreen_h__
#define TestDisplacementMappingScreen_h__

#include <Vorb/ui/IGameScreen.h>
#include <Vorb/graphics/GLProgram.h>

#include "Camera.h"

class TestDisplacementMappingScreen : public vui::IGameScreen
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
    Camera m_camera;
    vg::GLProgram m_program;
    VGTexture m_diffuseTexture;
    VGTexture m_normalTexture;
    VGTexture m_displacementTexture;
    float m_displacementScale;

    AutoDelegatePool m_hooks;
    bool m_ldown;
    f32v3 m_view;
};

#endif