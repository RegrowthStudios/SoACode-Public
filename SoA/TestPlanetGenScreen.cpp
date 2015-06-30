#include "stdafx.h"
#include "TestPlanetGenScreen.h"

#include "soaUtils.h"
#include "SpaceSystemAssemblages.h"
#include "Errors.h"
#include "ShaderLoader.h"
#include "SoaEngine.h"
#include <Vorb\graphics\GLProgram.h>
#include <Vorb\graphics\GpuMemory.h>
#include <Vorb\graphics\ImageIO.h>
#include <Vorb\io\IOManager.h>
#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/Timing.h>

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

i32 TestPlanetGenScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestPlanetGenScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestPlanetGenScreen::build() {

}
void TestPlanetGenScreen::destroy(const vui::GameTime& gameTime) {

}

void TestPlanetGenScreen::onEntry(const vui::GameTime& gameTime) {

    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s, const vui::KeyEvent& e) {
        if (e.keyCode == VKEY_F1) {
            m_terrainRenderer.dispose();
            m_terrainRenderer.initGL();
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onWheel, [&](Sender s, const vui::MouseWheelEvent& e) {
        m_eyeDist += -e.dy * 0.025 * glm::length(m_eyeDist);
    });
    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s, const vui::KeyEvent& e) {
        if (e.keyCode == VKEY_ESCAPE) {
            exit(0);
        }
    });
    glEnable(GL_DEPTH_TEST);
    glClearColor(1, 0, 0, 1);
    glClearDepth(1.0);

    m_terrainRenderer.initGL();
    m_atmoRenderer.initGL();

    SoaEngine::initState(&m_state);

    m_eyePos = f32v3(0, 0, m_eyeDist);

    // Set up planet
    m_state.spaceSystem->normalMapGenProgram = ShaderLoader::createProgramFromFile("Shaders/Generation/NormalMap.vert", "Shaders/Generation/NormalMap.frag");
    SystemBodyKegProperties props;
    PlanetKegProperties pProps;
    pProps.diameter = PLANET_RADIUS * 2.0;
    pProps.mass = 10000.0;
    PlanetGenData* genData = new PlanetGenData;
    TerrainFuncKegProperties tprops;
    tprops.low = 9;
    tprops.high = 10;
    genData->radius = PLANET_RADIUS;
    genData->baseTerrainFuncs.funcs.setData(&tprops, 1);
    pProps.planetGenData = genData;

    // Set up components
    SpaceSystemAssemblages::createPlanet(m_state.spaceSystem, &props, &pProps, &body);

    m_aCmp.radius = (f32)(PLANET_RADIUS * 1.025);
    m_aCmp.planetRadius = (f32)PLANET_RADIUS;
    m_aCmp.invWavelength4 = f32v3(1.0f / powf(0.475f, 4.0f),
                                  1.0f / powf(0.57f, 4.0f),
                                  1.0f / powf(0.65f, 4.0f));

    m_camera.setFieldOfView(90.0f);
    f32 width = (f32)m_game->getWindow().getWidth();
    f32 height = (f32)m_game->getWindow().getHeight();
    m_camera.setAspectRatio(width / height);
    m_camera.setDirection(f32v3(0.0f, 0.0f, -1.0f));
    m_camera.setUp(f32v3(0.0f, 1.0f, 0.0f));
}

void TestPlanetGenScreen::onExit(const vui::GameTime& gameTime) {

}

void TestPlanetGenScreen::update(const vui::GameTime& gameTime) {
    m_eyePos = f64v3(0, 0, PLANET_RADIUS + m_eyeDist + 100.0);

    m_updater.update(&m_state, m_eyePos);
    m_updater.glUpdate(&m_state);
}

void TestPlanetGenScreen::draw(const vui::GameTime& gameTime) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_camera.setClippingPlane((f32)(m_eyeDist / 2.0), (f32)(m_eyeDist + PLANET_RADIUS * 10.0));
    m_camera.setPosition(f64v3(m_eyePos));
    m_camera.update();

    f32v3 lightPos = glm::normalize(f32v3(0.0f, 0.0f, 1.0f));

    PreciseTimer timer;
    m_terrainRenderer.draw(m_state.spaceSystem->m_sphericalTerrainCT.getFromEntity(body.entity), &m_camera, lightPos,
                           f64v3(0.0f/*m_eyePos*/), computeZCoef(m_camera.getFarClip()), &m_slCmp,
                           &m_arCmp, &m_aCmp);

    m_atmoRenderer.draw(m_state.spaceSystem->m_atmosphereCT.getFromEntity(body.entity), m_camera.getViewProjectionMatrix(), f32v3(m_eyePos), lightPos,
                        computeZCoef(m_camera.getFarClip()), &m_slCmp);
    //glFinish();

    checkGlError("TestGasGiantScreen::draw");
}
