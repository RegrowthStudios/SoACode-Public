///
/// TestPlanetGenScreen.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 26 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Screen for testing planet generation
///

#pragma once

#ifndef TestPlanetGenScreen_h__
#define TestPlanetGenScreen_h__

#include "AtmosphereComponentRenderer.h"
#include "Camera.h"
#include "SphericalTerrainComponentRenderer.h"
#include "SpaceSystemComponents.h"
#include "SpaceSystem.h"
#include <Vorb/Events.hpp>
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/ui/IGameScreen.h>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/graphics/TextureCache.h>
#include <Vorb/Events.hpp>

#include <vector>

class TestPlanetGenScreen : public vui::IGameScreen {
public:
    /************************************************************************/
    /* IGameScreen functionality                                            */
    /************************************************************************/
    virtual i32 getNextScreen() const override;
    virtual i32 getPreviousScreen() const override;
    virtual void build() override;
    virtual void destroy(const vui::GameTime& gameTime) override;
    virtual void onEntry(const vui::GameTime& gameTime) override;
    virtual void onExit(const vui::GameTime& gameTime) override;
    virtual void update(const vui::GameTime& gameTime) override;
    virtual void draw(const vui::GameTime& gameTime) override;

private:
    const f64 PLANET_RADIUS = 154190.0 / 2.0;
    SphericalTerrainComponentRenderer m_terrainRenderer;
    AtmosphereComponentRenderer m_atmoRenderer;
    AxisRotationComponent m_arCmp;
    NamePositionComponent m_npCmp;
    SystemBody body;
    SpaceSystem m_spaceSystem;
    f64v3 m_eyePos;
    f64 m_eyeDist = PLANET_RADIUS;
    SphericalTerrainComponent m_stCmp;
    SpaceLightComponent m_slCmp;
    AtmosphereComponent m_aCmp;
    Camera m_camera;
    AutoDelegatePool m_hooks;
};

#endif // TestPlanetGenScreen_h__