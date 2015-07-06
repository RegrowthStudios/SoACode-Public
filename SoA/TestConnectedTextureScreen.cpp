#include "stdafx.h"
#include "TestConnectedTextureScreen.h"

#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/colors.h>

#include "SoaState.h"
#include "SoaEngine.h"
#include "LoadTaskBlockData.h"
#include "ChunkRenderer.h"

#include "ChunkMeshTask.h"

TestConnectedTextureScreen::TestConnectedTextureScreen(const App* app, CommonState* state) :
IAppScreen<App>(app),
m_commonState(state),
m_soaState(m_commonState->state) {

}

i32 TestConnectedTextureScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestConnectedTextureScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestConnectedTextureScreen::build() {

}

void TestConnectedTextureScreen::destroy(const vui::GameTime& gameTime) {

}

void TestConnectedTextureScreen::onEntry(const vui::GameTime& gameTime) {
    // Init spritebatch and font
    m_sb.init();
    m_font.init("Fonts/orbitron_bold-webfont.ttf", 32);
    // Init game state
    SoaEngine::initState(m_commonState->state);

    // Init renderer
    m_renderer.init();

    // Load blocks
    LoadTaskBlockData blockLoader(&m_soaState->blocks,
                                  &m_soaState->blockTextureLoader,
                                  &m_commonState->loadContext);
    blockLoader.load();

    // Uploads all the needed textures
    m_soaState->blockTextures->update();

    initChunks();

    // Create all chunk meshes
    m_mesher.init(&m_soaState->blocks);
    for (auto& cv : m_chunks) {
        cv.chunkMesh = m_mesher.easyCreateChunkMesh(cv.chunk, MeshTaskType::DEFAULT);
    }

    { // Init the camera
        m_camera.init(m_commonState->window->getAspectRatio());
        m_camera.setPosition(f64v3(16.0, 17.0, 33.0));
        m_camera.setDirection(f32v3(0.0f, 0.0f, -1.0f));
        m_camera.setRight(f32v3(1.0f, 0.0f, 0.0f));
        m_camera.setUp(f32v3(0.0f, 1.0f, 0.0f));
    }

    initInput();

    // Set GL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearDepth(1.0);
}

void TestConnectedTextureScreen::onExit(const vui::GameTime& gameTime) {
    for (auto& cv : m_chunks) {
        m_mesher.freeChunkMesh(cv.chunkMesh);
    }
}

void TestConnectedTextureScreen::update(const vui::GameTime& gameTime) {
    f32 speed = 5.0f;
    if (m_movingFast) speed *= 5.0f;
    if (m_movingForward) {
        f32v3 offset = m_camera.getDirection() * speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    if (m_movingBack) {
        f32v3 offset = m_camera.getDirection() * -speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    if (m_movingLeft) {
        f32v3 offset = m_camera.getRight() * -speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    if (m_movingRight) {
        f32v3 offset = m_camera.getRight() * speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    if (m_movingUp) {
        f32v3 offset = f32v3(0, 1, 0) * speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    if (m_movingDown) {
        f32v3 offset = f32v3(0, 1, 0) *  -speed * (f32)gameTime.elapsed;
        m_camera.offsetPosition(offset);
    }
    m_camera.update();
}

void TestConnectedTextureScreen::draw(const vui::GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    m_renderer.beginOpaque(m_soaState->blockTextures->getAtlasTexture(),
                           f32v3(0.0f, 0.0f, -1.0f), f32v3(1.0f),
                           f32v3(0.3f));
    m_renderer.drawOpaque(m_chunks[m_activeChunk].chunkMesh, m_camera.getPosition(), m_camera.getViewProjectionMatrix());
    m_renderer.end();

    if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Draw UI
    m_sb.begin();
    m_sb.drawString(&m_font, (std::to_string(m_activeChunk) + ". " + m_chunks[m_activeChunk].name).c_str(), f32v2(30.0f), f32v2(1.0f), color::White);
    m_sb.end();
    m_sb.render(f32v2(m_commonState->window->getViewportDims()));
    vg::DepthState::FULL.set(); // Have to restore depth
}

void TestConnectedTextureScreen::initChunks() {
    ui16 grass = m_soaState->blocks.getBlockIndex("grass");
    // TODO(Ben): Allow users to pick block
    { // Grass 1
        Chunk* chunk = addChunk("Grass 1");
        chunk->setBlock(15, 16, 16, grass);
        chunk->setBlock(16, 16, 16, grass);
        chunk->setBlock(17, 16, 16, grass);
        chunk->setBlock(15, 15, 17, grass);
        chunk->setBlock(16, 15, 17, grass);
        chunk->setBlock(17, 15, 17, grass);
        chunk->setBlock(15, 15, 18, grass);
        chunk->setBlock(16, 14, 18, grass);
        chunk->setBlock(17, 14, 18, grass);
        chunk->setBlock(15, 13, 19, grass);
        chunk->setBlock(16, 13, 19, grass);
        chunk->setBlock(17, 14, 19, grass);
    }
    { // Hourglass
        Chunk* chunk = addChunk("Hourglass");
        for (int y = 0; y < 16; y++) {
            for (int z = y; z < CHUNK_WIDTH - y; z++) {
                for (int x = y; x < CHUNK_WIDTH - y; x++) {
                    chunk->setBlock(x, y, z, grass);
                    chunk->setBlock(x, CHUNK_WIDTH - y - 1, z, grass);
                }
            }
        }
    }
    { // Flat
        Chunk* chunk = addChunk("Flat");
        for (int i = 0; i < CHUNK_LAYER; i++) {
            chunk->blocks.set(CHUNK_LAYER * 15 + i, grass);
        }
    }
}

void TestConnectedTextureScreen::initInput() {
    m_mouseButtons[0] = false;
    m_mouseButtons[1] = false;
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [&](Sender s, const vui::MouseMotionEvent& e) {
        if (m_mouseButtons[0]) {
            m_camera.rotateFromMouse(-e.dx, -e.dy, 0.1f);
        }
        if (m_mouseButtons[1]) {
            m_camera.rollFromMouse((f32)e.dx, 0.1f);
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s, const vui::MouseButtonEvent& e) {
        if (e.button == vui::MouseButton::LEFT) m_mouseButtons[0] = true;
        if (e.button == vui::MouseButton::RIGHT) m_mouseButtons[1] = true;
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [&](Sender s, const vui::MouseButtonEvent& e) {
        if (e.button == vui::MouseButton::LEFT) m_mouseButtons[0] = false;
        if (e.button == vui::MouseButton::RIGHT) m_mouseButtons[1] = false;
    });
    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s, const vui::KeyEvent& e) {
        switch (e.keyCode) {
            case VKEY_W:
                m_movingForward = true;
                break;
            case VKEY_S:
                m_movingBack = true;
                break;
            case VKEY_A:
                m_movingLeft = true;
                break;
            case VKEY_D:
                m_movingRight = true;
                break;
            case VKEY_SPACE:
                m_movingUp = true;
                break;
            case VKEY_LSHIFT:
                m_movingFast = true;
                break;
            case VKEY_M:
                m_wireFrame = !m_wireFrame;
                break;
            case VKEY_LEFT:
                if (m_activeChunk == 0) {
                    m_activeChunk = m_chunks.size() - 1;
                } else {
                    m_activeChunk--;
                }
                break;
            case VKEY_RIGHT:
                m_activeChunk++;
                if (m_activeChunk >= m_chunks.size()) m_activeChunk = 0;
                break;
            case VKEY_F10:
                // Reload meshes
                // TODO(Ben): Destroy meshes
                for (auto& cv : m_chunks) {
                    m_mesher.freeChunkMesh(cv.chunkMesh);
                    cv.chunkMesh = m_mesher.easyCreateChunkMesh(cv.chunk, MeshTaskType::DEFAULT);
                }
                break;
            case VKEY_F11:
                // Reload shaders
                m_renderer.dispose();
                m_renderer.init();
                break;
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyUp, [&](Sender s, const vui::KeyEvent& e) {
        switch (e.keyCode) {
            case VKEY_W:
                m_movingForward = false;
                break;
            case VKEY_S:
                m_movingBack = false;
                break;
            case VKEY_A:
                m_movingLeft = false;
                break;
            case VKEY_D:
                m_movingRight = false;
                break;
            case VKEY_SPACE:
                m_movingUp = false;
                break;
            case VKEY_LSHIFT:
                m_movingFast = false;
                break;
        }
    });
}

Chunk* TestConnectedTextureScreen::addChunk(const nString& name) {
    Chunk* chunk = new Chunk;
    chunk->initAndFillEmpty(0, ChunkPosition3D());
    // TODO(Ben): AOS
    ViewableChunk viewable;
    viewable.chunk = chunk;
    viewable.name = name;
    m_chunks.push_back(viewable);
    return chunk;
}