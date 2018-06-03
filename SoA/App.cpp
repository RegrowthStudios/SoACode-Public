#include "stdafx.h"
#include "App.h"

#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/ui/ScreenList.h>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/sound/SoundEngine.h>

#include "ChunkMeshManager.h"
#include "CommonState.h"
#include "DebugRenderer.h"
#include "DevScreen.h"
#include "GameManager.h"
#include "GameplayLoadScreen.h"
#include "GameplayScreen.h"
#include "InitScreen.h"
#include "MainMenuLoadScreen.h"
#include "MainMenuScreen.h"
#include "SoaEngine.h"
#include "SoaOptions.h"
#include "SoaState.h"
#include "SpaceSystem.h"
#include "TestBiomeScreen.h"
#include "TestBlockViewScreen.h"
#include "TestConnectedTextureScreen.h"
#include "TestConsoleScreen.h"
#include "TestDeferredScreen.h"
#include "TestDisplacementMappingScreen.h"
#include "TestGasGiantScreen.h"
#include "TestMappingScreen.h"
#include "TestNewBlockAPIScreen.h"
#include "TestNoiseScreen.h"
#include "TestPlanetGenScreen.h"
#include "TestStarScreen.h"
#include "TestVoxelModelScreen.h"

void App::addScreens() {
    scrInit = new InitScreen(this);  
    scrMainMenu = new MainMenuScreen(this, &state);
    scrLoad = new MainMenuLoadScreen(this, &state, scrMainMenu);
    scrGamePlay = new GameplayScreen(this, scrMainMenu);
    scrGameplayLoad = new GameplayLoadScreen(this, &state, scrMainMenu, scrGamePlay);

    m_screenList.addScreen(scrInit);
    m_screenList.addScreen(scrLoad);
    m_screenList.addScreen(scrMainMenu);
    m_screenList.addScreen(scrGameplayLoad);
    m_screenList.addScreen(scrGamePlay);

    // Add development screen
    scrDev = new DevScreen;
    scrDev->addScreen(VKEY_RETURN, scrInit, "Seed of Andromeda");
    m_screenList.addScreen(scrDev);

    // Add test screens
    scrTests.push_back(new TestConsoleScreen);
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_C, scrTests.back(), "TestConsoleScreen");
    scrTests.push_back(new TestMappingScreen);
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_M, scrTests.back(), "TestMappingScreen");
    scrTests.push_back(new TestDeferredScreen);
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_D, scrTests.back(), "TestDeferredScreen");
    scrTests.push_back(new TestBlockView);
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_B, scrTests.back(), "TestBlockView");
    scrTests.push_back(new TestGasGiantScreen);
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_G, scrTests.back(), "TestGasGiantScreen");
    scrTests.push_back(new TestStarScreen(this));
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_S, scrTests.back(), "TestStarScreen");
    scrTests.push_back(new TestVoxelModelScreen(this));
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_V, scrTests.back(), "TestVoxelModelScreen");
    scrTests.push_back(new TestDisplacementMappingScreen);
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_P, scrTests.back(), "TestDisplacementMappingScreen");
    scrTests.push_back(new TestNoiseScreen(this));
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_N, scrTests.back(), "TestNoiseScreen");
    scrTests.push_back(new TestNewBlockAPIScreen);
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_A, scrTests.back(), "TestNewBlockAPIScreen");
    scrTests.push_back(new TestPlanetGenScreen);
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_O, scrTests.back(), "TestPlanetGenScreen");
    scrTests.push_back(new TestConnectedTextureScreen(this, &state));
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_T, scrTests.back(), "TestConnectedTextureScreen");
    scrTests.push_back(new TestBiomeScreen(this, &state));
    m_screenList.addScreen(scrTests.back());
    scrDev->addScreen(VKEY_Z, scrTests.back(), "TestBiomeScreen");

    // Uncomment to start from dev screen for testing other screens
#define START_AT_DEV_SCREEN
#ifdef START_AT_DEV_SCREEN
    m_screenList.setScreen(scrDev->getIndex());
#else
    m_screenList.setScreen(scrInit->getIndex());
#endif
}

void App::onInit() {
    state.state = new SoaState;
    state.window = &m_window;
    state.soundEngine = new vsound::Engine;
    state.soundEngine->init();

    // Load the game options
    SoaEngine::initOptions(soaOptions);

    // Set the window options
    soaOptions.get(OPT_FULLSCREEN).value.b = m_window.isFullscreen();
    soaOptions.get(OPT_BORDERLESS).value.b = m_window.isBorderless();
    soaOptions.get(OPT_SCREEN_WIDTH).value.i = m_window.getWidth();
    soaOptions.get(OPT_SCREEN_HEIGHT).value.i = m_window.getHeight();
    soaOptions.get(OPT_VSYNC).value.i = (m_window.getSwapInterval() == vui::GameSwapInterval::V_SYNC);

    // Load the options from file
    SoaEngine::optionsController.loadOptions();

    vg::SamplerState::initPredefined();
}

void App::onExit() {
    // Delete cache if it exists

    vg::SpriteBatch::disposeProgram();
}

App::~App() {
    delete scrInit;
    delete scrLoad;
    delete scrMainMenu;
    delete scrGamePlay;
    delete scrDev;
}