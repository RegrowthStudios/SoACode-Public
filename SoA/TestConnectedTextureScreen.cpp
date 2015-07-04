#include "stdafx.h"
#include "TestConnectedTextureScreen.h"

#include <Vorb/ui/InputDispatcher.h>

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

    { // Create Chunks
        Chunk* chunk = new Chunk;
        chunk->initAndFillEmpty(0, ChunkPosition3D());
        for (int i = 0; i < CHUNK_LAYER; i++) {
            chunk->blocks.set(CHUNK_LAYER * 15 + i, m_soaState->blocks.getBlockIndex("grass"));
        }

        m_chunks.emplace_back(chunk);
    }

    // Create all chunk meshes
    m_mesher.init(&m_soaState->blocks);
    for (auto& chunk : m_chunks) {
        m_meshes.emplace_back(m_mesher.easyCreateChunkMesh(chunk, MeshTaskType::DEFAULT));
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
    for (auto& cm : m_meshes) {
        delete cm;
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
    m_renderer.drawOpaque(m_meshes[m_activeChunk], m_camera.getPosition(), m_camera.getViewProjectionMatrix());
    m_renderer.end();

    if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
                for (int i = 0; i < m_chunks.size(); i++) {
                    Chunk* chunk = m_chunks[i];
                    delete m_meshes[i];
                    m_meshes[i] = m_mesher.easyCreateChunkMesh(chunk, MeshTaskType::DEFAULT);
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