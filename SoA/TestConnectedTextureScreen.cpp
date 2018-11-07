#include "stdafx.h"
#include "TestConnectedTextureScreen.h"

#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/colors.h>

#include "SoAState.h"
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

void TestConnectedTextureScreen::destroy(const vui::GameTime& gameTime VORB_UNUSED) {

}

void TestConnectedTextureScreen::onEntry(const vui::GameTime& gameTime VORB_MAYBE_UNUSED) {

    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyUp, [&](Sender s VORB_MAYBE_UNUSED, const vui::KeyEvent& e) {
        if(e.keyCode==VKEY_ESCAPE)
        {
            exit(0);
        }
    });

    // Init spritebatch and font
    m_sb.init();
    m_font.init("Fonts/orbitron_bold-webfont.ttf", 32);
    // Init game state
    SoaEngine::initState(m_commonState->state);

    // Init renderer
    m_renderer.init();

    { // Init post processing
        Array<vg::GBufferAttachment> attachments;
        vg::GBufferAttachment att[2];
        // Color
        att[0].format = vg::TextureInternalFormat::RGBA16F;
        att[0].pixelFormat = vg::TextureFormat::RGBA;
        att[0].pixelType = vg::TexturePixelType::UNSIGNED_BYTE;
        att[0].number = 1;
        // Normals
        att[1].format = vg::TextureInternalFormat::RGBA16F;
        att[1].pixelFormat = vg::TextureFormat::RGBA;
        att[1].pixelType = vg::TexturePixelType::UNSIGNED_BYTE;
        att[1].number = 2;
        m_hdrTarget.setSize(m_commonState->window->getWidth(), m_commonState->window->getHeight());
        m_hdrTarget.init(Array<vg::GBufferAttachment>(att, 2), vg::TextureInternalFormat::RGBA8).initDepth();
    
        // Swapchain
        m_swapChain.init(m_commonState->window->getWidth(), m_commonState->window->getHeight(), vg::TextureInternalFormat::RGBA16F);
   
        // Init the FullQuadVBO
        m_commonState->quad.init();

        // SSAO
        m_ssaoStage.init(m_commonState->window, m_commonState->loadContext);
        m_ssaoStage.load(m_commonState->loadContext);
        m_ssaoStage.hook(&m_commonState->quad, m_commonState->window->getWidth(), m_commonState->window->getHeight());

        // HDR
        m_hdrStage.init(m_commonState->window, m_commonState->loadContext);
        m_hdrStage.load(m_commonState->loadContext);
        m_hdrStage.hook(&m_commonState->quad);
    }

    // Load blocks
    LoadTaskBlockData blockLoader(&m_soaState->blocks,
                                  &m_soaState->clientState.blockTextureLoader,
                                  &m_commonState->loadContext);
    blockLoader.load();

    // Uploads all the needed textures
    m_soaState->clientState.blockTextures->update();

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

void TestConnectedTextureScreen::onExit(const vui::GameTime& gameTime VORB_MAYBE_UNUSED) {
    for (auto& cv : m_chunks) {
        m_mesher.freeChunkMesh(cv.chunkMesh);
    }
    m_hdrTarget.dispose();
    m_swapChain.dispose();
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

void TestConnectedTextureScreen::draw(const vui::GameTime& gameTime VORB_UNUSED) {
    // Bind the FBO
    m_hdrTarget.useGeometry();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    m_renderer.beginOpaque(m_soaState->clientState.blockTextures->getAtlasTexture(),
                           f32v3(0.0f, 0.0f, -1.0f), f32v3(1.0f),
                           f32v3(0.3f));
    m_renderer.drawOpaque(m_chunks[m_activeChunk].chunkMesh, m_camera.getPosition(), m_camera.getViewProjectionMatrix());
    m_renderer.end();

    if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Post processing
    m_swapChain.reset(0, m_hdrTarget.getGeometryID(), m_hdrTarget.getGeometryTexture(0), soaOptions.get(OPT_MSAA).value.i > 0, false);

    // Render SSAO
    m_ssaoStage.set(m_hdrTarget.getDepthTexture(), m_hdrTarget.getGeometryTexture(1), m_hdrTarget.getGeometryTexture(0), m_swapChain.getCurrent().getID());
    m_ssaoStage.render(&m_camera);
    m_swapChain.swap();
    m_swapChain.use(0, false);

    // Draw to backbuffer for the last effect
    // TODO(Ben): Do we really need to clear depth here...
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_hdrTarget.getDepthTexture());
    m_hdrStage.render();

    // Draw UI
    m_sb.begin();
    m_sb.drawString(&m_font, (std::to_string(m_activeChunk) + ". " + m_chunks[m_activeChunk].name).c_str(), f32v2(30.0f), f32v2(1.0f), color::White);
    m_sb.end();
    m_sb.render(f32v2(m_commonState->window->getViewportDims()));
    vg::DepthState::FULL.set(); // Have to restore depth
}

void TestConnectedTextureScreen::initChunks() {
    ui16 grass = m_soaState->blocks.getBlockIndex("grass");
    ui16 dirt = m_soaState->blocks.getBlockIndex("dirt");
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
        for (int y = 0; y < HALF_CHUNK_WIDTH; y++) {
            for (int z = y; z < CHUNK_WIDTH - y; z++) {
                for (int x = y; x < CHUNK_WIDTH - y; x++) {
                    chunk->setBlock(x, y, z, grass);
                    chunk->setBlock(x, CHUNK_WIDTH - y - 1, z, grass);
                }
            }
        }
        for (int y = 0; y < CHUNK_WIDTH; y++) {
            chunk->setBlock(0, y, 0, dirt);
            chunk->setBlock(CHUNK_WIDTH_M1, y, CHUNK_WIDTH_M1, dirt);
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
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [&](Sender s VORB_MAYBE_UNUSED, const vui::MouseMotionEvent& e) {
        if (m_mouseButtons[0]) {
            m_camera.rotateFromMouse((f32)-e.dx, (f32)-e.dy, 0.1f);
        }
        if (m_mouseButtons[1]) {
            m_camera.rollFromMouse((f32)e.dx, 0.1f);
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s VORB_MAYBE_UNUSED, const vui::MouseButtonEvent& e) {
        if (e.button == vui::MouseButton::LEFT) m_mouseButtons[0] = true;
        if (e.button == vui::MouseButton::RIGHT) m_mouseButtons[1] = true;
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonUp, [&](Sender s VORB_MAYBE_UNUSED, const vui::MouseButtonEvent& e) {
        if (e.button == vui::MouseButton::LEFT) m_mouseButtons[0] = false;
        if (e.button == vui::MouseButton::RIGHT) m_mouseButtons[1] = false;
    });
    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s VORB_MAYBE_UNUSED, const vui::KeyEvent& e) {
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
                if (m_activeChunk >= (int)m_chunks.size()) m_activeChunk = 0;
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
                m_ssaoStage.reloadShaders();
                break;
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyUp, [&](Sender s VORB_MAYBE_UNUSED, const vui::KeyEvent& e) {
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
    // TODO(Ben): This is wrong now, need accessor
    chunk->initAndFillEmpty(WorldCubeFace::FACE_TOP);
    // TODO(Ben): AOS
    ViewableChunk viewable;
    viewable.chunk = chunk;
    viewable.name = name;
    m_chunks.push_back(viewable);
    return chunk;
}