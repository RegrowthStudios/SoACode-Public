#include "stdafx.h"
#include "LoadScreen.h"

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
#include "LoadTaskBlockData.h"
#include "LoadTaskGameManager.h"
#include "LoadTaskStarSystem.h"
#include "LoadTaskTextures.h"
#include "MainMenuScreen.h"
#include "MeshManager.h"
#include "MusicPlayer.h"
#include "ParticleEmitter.h"
#include "SoaFileSystem.h"
#include "SoaState.h"

#include "TexturePackLoader.h"

const color4 LOAD_COLOR_TEXT(205, 205, 205, 255);
const color4 LOAD_COLOR_BG_LOADING(105, 5, 5, 255);
const color4 LOAD_COLOR_BG_FINISHED(25, 105, 5, 255);

LoadScreen::LoadScreen(const App* app, CommonState* state, MainMenuScreen* mainMenuScreen) :
IAppScreen<App>(app),
m_commonState(state),
m_sf(nullptr),
m_sb(nullptr),
m_monitor(),
m_glrpc(),
m_mainMenuScreen(mainMenuScreen) {
    // Empty
}


i32 LoadScreen::getNextScreen() const {
    return m_app->scrMainMenu->getIndex();
}
i32 LoadScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void LoadScreen::build() {
    m_env.addValue("WindowWidth", (f64)m_game->getWindow().getWidth());
    m_env.addValue("WindowHeight", (f64)m_game->getWindow().getHeight());
    m_env.load("Data/Logos/Vorb/ScreenUpdate.lua");
    m_screenDuration = (m_env["Screen.MaxDuration"].as<f64>())();
    m_fUpdatePosition = m_env["Screen.PositionAtTime"].as<f32v2>();
    m_fUpdateColor = m_env["Screen.ColorAtTime"].as<f32v4>();

    { // Load all textures
        static cString texturePaths[VORB_NUM_TEXTURES] = {
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
            vg::Texture& tex = m_textures[i];

            // Load file
            vg::BitmapResource bmp = imageIO.load(texturePaths[i]);
            tex.width = bmp.width;
            tex.height = bmp.height;

            // Create GPU texture
            glGenTextures(1, &tex.id);
            glBindTexture(GL_TEXTURE_2D, tex.id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp.data);
            vg::SamplerState::LINEAR_CLAMP.set(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            vg::ImageIO::free(bmp);
        }
    }
}
void LoadScreen::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void LoadScreen::onEntry(const vui::GameTime& gameTime) {
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

    addLoadTask("SpaceSystem", "SpaceSystem", new LoadTaskStarSystem(&m_glrpc, "StarSystems/Trinity", m_commonState->state));
    m_monitor.setDep("SpaceSystem", "GameManager");

    m_mainMenuScreen->m_renderer.init(m_commonState->window, m_commonState->loadContext, m_mainMenuScreen);
    m_mainMenuScreen->m_renderer.hook(m_commonState->state);
    m_mainMenuScreen->m_renderer.load(m_commonState->loadContext);

    // Start the tasks
    m_monitor.start();

    // Clear State For The Screen
    f32v4 clearColor = m_env["Screen.BackgroundColor"].as<f32v4>()();
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClearDepth(1.0);

    m_isSkipDetected = false;
    m_timer = 0.0;
    vui::InputDispatcher::key.onKeyDown += makeDelegate(*this, &LoadScreen::onKeyPress);
}
void LoadScreen::onExit(const vui::GameTime& gameTime) {
    m_sf->dispose();
    delete m_sf;
    m_sf = nullptr;

    m_sb->dispose();
    delete m_sb;
    m_sb = nullptr;

    // Free the vector memory
    std::vector<LoadBar>().swap(m_loadBars);

    for (ui32 i = 0; i < m_loadTasks.size(); i++) {
        // Free memory
        delete m_loadTasks[i];
        m_loadTasks[i] = nullptr;
    }
    std::vector<ILoadTask*>().swap(m_loadTasks);

    // Restore default rasterizer state
    vg::RasterizerState::CULL_CLOCKWISE.set();

    vui::InputDispatcher::key.onKeyDown -= makeDelegate(*this, &LoadScreen::onKeyPress);
}

void LoadScreen::update(const vui::GameTime& gameTime) {
    static ui64 fCounter = 0;

    // Increment elapsed time
    m_timer += gameTime.elapsed;

    for (ui32 i = 0; i < m_loadTasks.size(); i++) {
        if (m_loadTasks[i] != nullptr && m_loadTasks[i]->isFinished()) {
            // Make The Task Visuals Disappear
            m_loadBars[i].setColor(color::Black, color::Teal);
            m_loadBars[i].retract();
        }

        // Update Visual Position
        m_loadBars[i].update((f32)gameTime.elapsed);
    }

    // Perform OpenGL calls
    fCounter++;
    m_glrpc.processRequests(1);
    m_mainMenuScreen->m_renderer.updateGL();

    // Defer texture loading
    static bool loadedTextures = false;
    if (m_mainMenuScreen->m_renderer.isLoaded() && m_monitor.isTaskFinished("SpaceSystem") && (m_isSkipDetected || m_timer > m_screenDuration)) {// && !loadedTextures && m_monitor.isTaskFinished("Textures")) {
        //GameManager::texturePackLoader->uploadTextures();
        //GameManager::texturePackLoader->writeDebugAtlases();
        //GameManager::texturePackLoader->setBlockTextures(Blocks);

        //GameManager::getTextureHandles();

        //SetBlockAvgTexColors();

        ////load the emitters
        //for (size_t i = 0; i < Blocks.size(); i++) {
        //    if (Blocks[i].active) {
        //        if (Blocks[i].emitterName.size()) {
        //        //    Blocks[i].emitter = fileManager.loadEmitter(Blocks[i].emitterName);
        //        }
        //        if (Blocks[i].emitterOnBreakName.size()) {
        //       //     Blocks[i].emitterOnBreak = fileManager.loadEmitter(Blocks[i].emitterOnBreakName);
        //        }
        //        if (Blocks[i].emitterRandomName.size()) {
        //        //    Blocks[i].emitterRandom = fileManager.loadEmitter(Blocks[i].emitterRandomName);
        //        }
        //    }
        //}

        //// It has no texture
        //for (i32 i = 0; i < 6; i++) Blocks[0].base[i] = -1;

        //// Post process the planets
        //SoaEngine::setPlanetBlocks(m_commonState->state);
        m_state = vui::ScreenState::CHANGE_NEXT;
        loadedTextures = true;
    
    }
}
void LoadScreen::draw(const vui::GameTime& gameTime) {
    static cString textureNames[VORB_NUM_TEXTURES] = {
        "V", "O", "R", "B", "CubeLeft", "CubeRight", "CubeTop"
    };
    const vui::GameWindow& w = m_game->getWindow();

    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw vorb logo
    // Update the window size
    f32v2 windowSize(w.getWidth(), w.getHeight());
    m_env.addValue("WindowWidth", windowSize.x);
    m_env.addValue("WindowHeight", windowSize.y);

    // Render each texture
    m_sb->begin();
    for (size_t i = 0; i < VORB_NUM_TEXTURES; i++) {
        f32v2 pos = m_fUpdatePosition(m_timer, nString(textureNames[i]));
        f32v4 color = m_fUpdateColor(m_timer, nString(textureNames[i]));
        f32v4 rect(0.01f, 0.01f, 0.98f, 0.98f);
        m_sb->draw(m_textures[i].id, &rect, nullptr, pos, f32v2(m_textures[i].width, m_textures[i].height), color4(color.r, color.g, color.b, color.a));
    }
    m_sb->end();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_sb->render(windowSize);

    // Draw loading stuff

    /*  m_sb->begin();
      for (ui32 i = 0; i < m_loadTasks.size(); i++) {
      m_loadBars[i].draw(m_sb, m_sf, 0, 0.8f);
      }

      m_sb->end(vg::SpriteSortMode::BACK_TO_FRONT);

      m_sb->render(f32v2(w->getWidth(), w->getHeight()), &vg::SamplerState::LINEAR_WRAP, &vg::DepthState::NONE, &vg::RasterizerState::CULL_NONE);*/
    checkGlError("LoadScreen::draw()");
    
}

void LoadScreen::addLoadTask(const nString& name, const cString loadText, ILoadTask* task) {
    // Add the load task to the monitor
    m_loadTasks.push_back(task);
    m_monitor.addTask(name, m_loadTasks.back());

    // Load bar properties
    LoadBarCommonProperties lbcp(f32v2(500, 0), f32v2(500, 60), 800.0f, f32v2(10, 10), 40.0f);
    // Add the new loadbar and get its index
    int i = m_loadBars.size();
    m_loadBars.emplace_back();

    // Set the properties
    m_loadBars[i].setCommonProperties(lbcp);
    m_loadBars[i].setStartPosition(f32v2(-lbcp.offsetLength, 30 + i * lbcp.size.y));
    m_loadBars[i].expand();
    m_loadBars[i].setColor(color::Black, color::Maroon);
    m_loadBars[i].setText(loadText);
}

void LoadScreen::onKeyPress(Sender, const vui::KeyEvent& e) {
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
