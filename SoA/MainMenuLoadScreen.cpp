#include "stdafx.h"
#include "MainMenuLoadScreen.h"

#include <Vorb/colors.h>
#include <Vorb/graphics/GLStates.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/graphics/SpriteBatch.h>

#include "App.h"
#include "BlockPack.h"
#include "ChunkMeshManager.h"
#include "DebugRenderer.h"
#include "GameManager.h"
#include "InputMapper.h"
#include "Inputs.h"
#include "LoadTaskGameManager.h"
#include "LoadTaskStarSystem.h"
#include "MainMenuScreen.h"
#include "MeshManager.h"
#include "MusicPlayer.h"
#include "ParticleEmitter.h"
#include "SoaFileSystem.h"
#include "SoaState.h"

// TMP
#include "SimplexNoise.h"
#include "CpuNoise.h"


#include "SmartVoxelContainer.hpp"

const color4 LOAD_COLOR_TEXT(205, 205, 205, 255);
const color4 LOAD_COLOR_BG_LOADING(105, 5, 5, 255);
const color4 LOAD_COLOR_BG_FINISHED(25, 105, 5, 255);

MainMenuLoadScreen::MainMenuLoadScreen(const App* app, CommonState* state, MainMenuScreen* mainMenuScreen) :
IAppScreen<App>(app),
m_commonState(state),
m_mainMenuScreen(mainMenuScreen) {
    // Empty
}

i32 MainMenuLoadScreen::getNextScreen() const {
    return m_app->scrMainMenu->getIndex();
}
i32 MainMenuLoadScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void MainMenuLoadScreen::build() {
    m_env.addValue("WindowWidth", (f64)m_game->getWindow().getWidth());
    m_env.addValue("WindowHeight", (f64)m_game->getWindow().getHeight());
    m_env.load("Data/Logos/Vorb/ScreenUpdate.lua");
    m_vorbScreenDuration = (m_env["Vorb.MaxDuration"].as<f64>())();
    m_fUpdateVorbPosition = m_env["Vorb.PositionAtTime"].as<f32v2>();
    m_fUpdateVorbColor = m_env["Vorb.ColorAtTime"].as<f32v4>();
    m_fUpdateVorbBackColor = m_env["Vorb.BackgroundColor"].as<f32v4>();
    m_env.load("Data/Logos/Regrowth/ScreenUpdate.lua");
    m_regrowthScreenDuration = (m_env["Regrowth.MaxDuration"].as<f64>())();
    m_fUpdateRegrowthPosition = m_env["Regrowth.PositionAtTime"].as<f32v2>();
    m_fUpdateRegrowthColor = m_env["Regrowth.ColorAtTime"].as<f32v4>();
    m_fUpdateRegrowthBackColor = m_env["Regrowth.BackgroundColor"].as<f32v4>();
    m_regrowthScale = (m_env["Regrowth.Scale"].as<f32>())();
    { // Load all textures
        const cString vorbTexturePaths[VORB_NUM_TEXTURES] = {
            "Data/Logos/Vorb/V.png",
            "Data/Logos/Vorb/O.png",
            "Data/Logos/Vorb/R.png",
            "Data/Logos/Vorb/B.png",
            "Data/Logos/Vorb/CubeLeft.png",
            "Data/Logos/Vorb/CubeRight.png",
            "Data/Logos/Vorb/CubeTop.png"
        };
        vg::ImageIO imageIO;
        for (size_t i = 0; i < VORB_NUM_TEXTURES; i++) {
            vg::Texture& tex = m_vorbTextures[i];

            // Load file
            vg::ScopedBitmapResource bmp = imageIO.load(vorbTexturePaths[i]);
            tex.width = bmp.width;
            tex.height = bmp.height;

            // Create GPU texture
            glGenTextures(1, &tex.id);
            glBindTexture(GL_TEXTURE_2D, tex.id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp.data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        const cString regrowthTexturePaths[REGROWTH_NUM_TEXTURES] = {
            "Data/Logos/Regrowth/Regrowth.png",
            "Data/Logos/Regrowth/Studios.png"
        };
        for (size_t i = 0; i < REGROWTH_NUM_TEXTURES; i++) {
            vg::Texture& tex = m_regrowthTextures[i];

            // Load file
            vg::ScopedBitmapResource bmp = imageIO.load(regrowthTexturePaths[i]);
            tex.width = bmp.width;
            tex.height = bmp.height;

            // Create GPU texture
            glGenTextures(1, &tex.id);
            glBindTexture(GL_TEXTURE_2D, tex.id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp.data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}
void MainMenuLoadScreen::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void MainMenuLoadScreen::onEntry(const vui::GameTime& gameTime) {

    //PreciseTimer timer;
    //f64v3 pos(6500 , 3344 , 6633 );
    //f32v3 pos2(6500, 3344, 6633);
    //timer.start();
    //volatile f64 total = 0.0;
    //timer.start();
    //f64 freq = 0.01;
    //f32 freq2 = 0.01;
    //f64 tot1 = 0.0;
    //f32 tot2 = 0.0f; 
    //for (int i = 0; i < 20000000; i++) {
    //    f64 a = raw_noise_3d(pos.x * freq, pos.y * freq, pos.z * freq);
    //    f32 b = raw_noise_3df(pos2.x * freq2, pos2.y * freq2, pos2.z * freq2);
    //    total += a + b;
    //    freq *= 2.0;
    //    freq2 *= 2.0f;
    //    tot1 += a;
    //    tot2 += b;
    //    if ((f32)tot1 != tot2) {
    //        printf("%d, %f, %f\n", i, (f32)tot1, tot2);
    //    }
    //}
    //std::cout << tot1 << " " << tot2 << std::endl;
    //printf("%lf ms\n", timer.stop());
    //std::cout << total;


    SoaFileSystem fs;
    fs.init();
    MusicPlayer mp;
    mp.refreshLists(fs);

    SoaEngine::initState(m_commonState->state);

    // Make LoadBar Resources
    m_sb = new vg::SpriteBatch(true, true);
    m_sf = new vg::SpriteFont();
    m_sf->init("Fonts/orbitron_bold-webfont.ttf", 32);

    // Add Tasks Here
    addLoadTask("GameManager", "Core Systems", new LoadTaskGameManager);

    addLoadTask("SpaceSystem", "SpaceSystem", new LoadTaskStarSystem(&m_commonState->loadContext.rpcManager, "StarSystems/Trinity", m_commonState->state));
    m_monitor.setDep("SpaceSystem", "GameManager");

    m_mainMenuScreen->m_renderer.init(m_commonState->window, m_commonState->loadContext, m_mainMenuScreen, m_commonState);
    m_mainMenuScreen->m_renderer.hook();
    m_commonState->loadContext.begin();
    m_mainMenuScreen->m_renderer.load(m_commonState->loadContext);

    // Start the tasks
    m_monitor.start();

    m_isSkipDetected = false;
    m_timer = 0.0;
    vui::InputDispatcher::key.onKeyDown += makeDelegate(*this, &MainMenuLoadScreen::onKeyPress);
}
void MainMenuLoadScreen::onExit(const vui::GameTime& gameTime) {
    m_sf->dispose();
    delete m_sf;
    m_sf = nullptr;

    m_sb->dispose();
    delete m_sb;
    m_sb = nullptr;

    for (ui32 i = 0; i < m_loadTasks.size(); i++) {
        // Free memory
        delete m_loadTasks[i];
        m_loadTasks[i] = nullptr;
    }
    std::vector<ILoadTask*>().swap(m_loadTasks);

    // Restore default rasterizer state
    vg::RasterizerState::CULL_CLOCKWISE.set();

    // Free textures
    for (size_t i = 0; i < VORB_NUM_TEXTURES; i++) glDeleteTextures(1, &m_vorbTextures[i].id);
    for (size_t i = 0; i < REGROWTH_NUM_TEXTURES; i++) glDeleteTextures(1, &m_regrowthTextures[i].id);

    vui::InputDispatcher::key.onKeyDown -= makeDelegate(*this, &MainMenuLoadScreen::onKeyPress);
}

void MainMenuLoadScreen::update(const vui::GameTime& gameTime) {
    // Increment elapsed time
    m_timer += gameTime.elapsed;
    if (m_isOnVorb && m_timer > m_vorbScreenDuration) {
        m_timer = 0.0f;
        m_isOnVorb = false;
    }

    // Perform OpenGL calls
    m_commonState->loadContext.processRequests(1);

    // End condition
    if (m_mainMenuScreen->m_renderer.isLoaded() && m_monitor.isTaskFinished("SpaceSystem") && (m_isSkipDetected || (!m_isOnVorb && m_timer > m_regrowthScreenDuration))) {
        m_commonState->loadContext.end();
        m_state = vui::ScreenState::CHANGE_NEXT;
    }
}
void MainMenuLoadScreen::draw(const vui::GameTime& gameTime) {
    static cString vorbTextureNames[VORB_NUM_TEXTURES] = {
        "V", "O", "R", "B", "CubeLeft", "CubeRight", "CubeTop"
    };
    static cString regrowthTextureNames[REGROWTH_NUM_TEXTURES] = {
        "Regrowth", "Studios"
    };
    const vui::GameWindow& w = m_game->getWindow();

    // Clear State For The Screen
    f32v4 clearColor;
    if (m_isOnVorb) {
        clearColor = m_fUpdateVorbBackColor(m_timer);
    } else {
        clearColor = m_fUpdateRegrowthBackColor(m_timer);
    }
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw vorb logo
    // Update the window size
    f32v2 windowSize(w.getWidth(), w.getHeight());
    m_env.addValue("WindowWidth", windowSize.x);
    m_env.addValue("WindowHeight", windowSize.y);

    // Render each texture
    m_sb->begin();
    f32v2 uvTile(0.999999f, 0.999999f);
    if (m_isOnVorb) {
        for (size_t i = 0; i < VORB_NUM_TEXTURES; i++) {
            f32v2 pos = m_fUpdateVorbPosition(m_timer, nString(vorbTextureNames[i]));
            f32v4 color = m_fUpdateVorbColor(m_timer, nString(vorbTextureNames[i]));          
            m_sb->draw(m_vorbTextures[i].id, nullptr, &uvTile, pos, f32v2(m_vorbTextures[i].width, m_vorbTextures[i].height), color4(color.r, color.g, color.b, color.a));
        }
    } else {
        if (m_timer <= m_regrowthScreenDuration) {
            for (size_t i = 0; i < REGROWTH_NUM_TEXTURES; i++) {
                f32v2 pos = m_fUpdateRegrowthPosition(m_timer, nString(regrowthTextureNames[i]));
                f32v4 color = m_fUpdateRegrowthColor(m_timer, nString(regrowthTextureNames[i]));
                m_sb->draw(m_regrowthTextures[i].id, nullptr, &uvTile, pos, f32v2(m_regrowthTextures[i].width, m_regrowthTextures[i].height) * m_regrowthScale, color4(color.r, color.g, color.b, color.a));
            }
        } else {
            // Animated loading message. TODO(Ben): Replace with progress bar
            int it = (int)m_timer % 3;
            if (it == 0) {
                m_sb->drawString(m_sf, "Loading.", f32v2(w.getWidth() - m_sf->measure("Loading").x * 0.5, w.getHeight()) / 2.0f, f32v2(1.0), color::White, vg::TextAlign::LEFT);
            } else if (it == 1) {
                m_sb->drawString(m_sf, "Loading..", f32v2(w.getWidth() - m_sf->measure("Loading").x * 0.5, w.getHeight()) / 2.0f, f32v2(1.0), color::White, vg::TextAlign::LEFT);
            } else {
                m_sb->drawString(m_sf, "Loading...", f32v2(w.getWidth() - m_sf->measure("Loading").x * 0.5, w.getHeight()) / 2.0f, f32v2(1.0), color::White, vg::TextAlign::LEFT);
            }
        }     
    }
    m_sb->end();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_sb->render(windowSize, &vg::SamplerState::LINEAR_CLAMP);

    // Draw progress
    f32 progress = m_commonState->loadContext.getPercentComplete();
    static f32 maxw = 48.0f;
    static f32 alpha = 1.0f;
    if (alpha > 0.0f) {
        const f32 border = 2.0f;
        const f32v2 pos(w.getWidth() - maxw * 2.0f, w.getHeight() - maxw * 2.0f);

        if (progress >= 1.0f) {
            alpha -= 0.02f;
            if (alpha < 0.0f) alpha = 0.0f;
        }
        f32 width = alpha * maxw;
        m_sb->begin();
        f32v2 size = f32v2(width - border * 2.0f);
        m_sb->draw(0, pos - size / 2.0f, size, color::Transparent, 0.1f);
        size = f32v2(width);
        m_sb->draw(0, pos - size / 2.0f, size, color4(169u, 169u, 169u, (ui8)(alpha * 255.0f)), 0.1f);
        size = f32v2(progress * width);
        m_sb->draw(0, pos - size / 2.0f, size, color4(16u, 190u, 239u, ui8(alpha * 255.0f)));
        m_sb->end(vg::SpriteSortMode::NONE);
        m_sb->render(f32v2(w.getWidth(), w.getHeight()), nullptr, &vg::DepthState::FULL);
    }
    checkGlError("LoadScreen::draw()");
    
}

void MainMenuLoadScreen::addLoadTask(const nString& name, const cString loadText, ILoadTask* task) {
    // Add the load task to the monitor
    m_loadTasks.push_back(task);
    m_monitor.addTask(name, m_loadTasks.back());
}

void MainMenuLoadScreen::onKeyPress(Sender, const vui::KeyEvent& e) {
    switch (e.keyCode) {
        case VKEY_RETURN:
        case VKEY_ESCAPE:
        case VKEY_SPACE:
            m_isSkipDetected = true;
            break;
        default:
            break;
    }
}
