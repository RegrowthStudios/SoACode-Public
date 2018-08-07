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
#include "SpaceSystem.h"
#include "SpaceSystemComponents.h"
#include "SpaceSystemUpdater.h"
#include "SphericalTerrainComponentRenderer.h"
#include "SphericalTerrainComponentUpdater.h"
#include "PlanetGenLoader.h"
#include "SoAState.h"
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
    const f64 PLANET_RADIUS = 6000.0;
    SphericalTerrainComponentRenderer m_terrainRenderer;
    AtmosphereComponentRenderer m_atmoRenderer;
    SystemBody body;
    f64v3 m_eyePos;
    f64 m_eyeDist = PLANET_RADIUS;
    SpaceSystemUpdater m_updater;
    SpaceLightComponent m_slCmp;
    Camera m_camera;
    AutoDelegatePool m_hooks;
    vio::IOManager m_iom;
    SoaState m_state;
};

#endif // TestPlanetGenScreen_h__