#include "stdafx.h"
#include "TestBiomeScreen.h"

#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/colors.h>

#include "App.h"
#include "ChunkRenderer.h"
#include "LoadTaskBlockData.h"
#include "SoaEngine.h"
#include "SoaState.h"

#define HORIZONTAL_CHUNKS 10
#define VERTICAL_CHUNKS 10

TestBiomeScreen::TestBiomeScreen(const App* app, CommonState* state) :
IAppScreen<App>(app),
m_commonState(state),
m_soaState(m_commonState->state) {

}

i32 TestBiomeScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestBiomeScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestBiomeScreen::build() {

}

void TestBiomeScreen::destroy(const vui::GameTime& gameTime) {

}

void TestBiomeScreen::onEntry(const vui::GameTime& gameTime) {
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
                                  &m_soaState->blockTextureLoader,
                                  &m_commonState->loadContext);
    blockLoader.load();

    // Uploads all the needed textures
    m_soaState->blockTextures->update();
    // Load test planet
    PlanetLoader planetLoader;
    m_iom.setSearchDirectory("StarSystems/Trinity/");
    planetLoader.init(&m_iom);
    m_genData = planetLoader.loadPlanetGenData("Planets/Aldrin/terrain_gen.yml");
    m_genData->radius = 4500.0;
    
    // Set blocks
    SoaEngine::setPlanetBlocks(m_genData, m_soaState->blocks);
    
    m_chunkGenerator.init(m_genData);

    printf("Generating chunks...");
    initChunks();

    printf("Generating Meshes...");
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
    printf("Done.");
}

void TestBiomeScreen::onExit(const vui::GameTime& gameTime) {
    for (auto& cv : m_chunks) {
        m_mesher.freeChunkMesh(cv.chunkMesh);
    }
    m_hdrTarget.dispose();
    m_swapChain.dispose();
}

void TestBiomeScreen::update(const vui::GameTime& gameTime) {
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

void TestBiomeScreen::draw(const vui::GameTime& gameTime) {
    // Bind the FBO
    m_hdrTarget.useGeometry();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    m_renderer.beginOpaque(m_soaState->blockTextures->getAtlasTexture(),
                           f32v3(0.0f, 0.0f, -1.0f), f32v3(1.0f),
                           f32v3(0.3f));
    for (auto& vc : m_chunks) {
        m_renderer.drawOpaque(vc.chunkMesh, m_camera.getPosition(), m_camera.getViewProjectionMatrix());
    }
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
    char buf[256];
    sprintf(buf, "FPS: %.1f", m_app->getFps());
    m_sb.begin();
    m_sb.drawString(&m_font, buf, f32v2(30.0f), f32v2(1.0f), color::White);
    m_sb.end();
    m_sb.render(f32v2(m_commonState->window->getViewportDims()));
    vg::DepthState::FULL.set(); // Have to restore depth
}

void TestBiomeScreen::initChunks() {
    ui16 grass = m_soaState->blocks.getBlockIndex("grass");
    ui16 dirt = m_soaState->blocks.getBlockIndex("dirt");
    
    m_chunks.resize(HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS * VERTICAL_CHUNKS);
    m_heightData.resize(HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS);

    // Init height data
    m_heightGenerator.init(m_genData);
    for (int z = 0; z < HORIZONTAL_CHUNKS; z++) {
        for (int x = 0; x < HORIZONTAL_CHUNKS; x++) {
            auto& hd = m_heightData[z * HORIZONTAL_CHUNKS + x];
            for (int i = 0; i < CHUNK_WIDTH; i++) {
                for (int j = 0; j < CHUNK_WIDTH; j++) {
                    VoxelPosition2D pos;
                    pos.pos.x = x * CHUNK_WIDTH + j;
                    pos.pos.y = z * CHUNK_WIDTH + i;
                    m_heightGenerator.generateHeightData(hd.heightData[i * CHUNK_WIDTH + j], pos);
                }
            }
        }
    }

    // Get center height
    f32 cHeight = m_heightData[HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS / 2].heightData[CHUNK_LAYER / 2].height;
    // Center the heightmap
    for (auto& hd : m_heightData) {
        for (int i = 0; i < CHUNK_LAYER; i++) {
            hd.heightData[i].height -= cHeight;
        }
    }

    for (int i = 0; i < m_chunks.size(); i++) {
        ChunkPosition3D pos;
        // Center position about origin
        pos.pos.x = i % HORIZONTAL_CHUNKS - HORIZONTAL_CHUNKS / 2;
        pos.pos.y = i / (HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS) - VERTICAL_CHUNKS / 2;
        pos.pos.z = (i % (HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS)) / HORIZONTAL_CHUNKS - HORIZONTAL_CHUNKS / 2;
        m_chunks[i].chunk = new Chunk;
        m_chunks[i].chunk->init(i, pos);
        m_chunkGenerator.generateChunk(m_chunks[i].chunk, m_heightData[i % (HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS)].heightData);
    }
}

void TestBiomeScreen::initInput() {
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
                /*    if (m_activeChunk == 0) {
                        m_activeChunk = m_chunks.size() - 1;
                        } else {
                        m_activeChunk--;
                        }*/
                break;
            case VKEY_RIGHT:
                /* m_activeChunk++;
                 if (m_activeChunk >= (int)m_chunks.size()) m_activeChunk = 0;*/
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
