#include "stdafx.h"
#include "TestBiomeScreen.h"

#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/colors.h>

#include "App.h"
#include "ChunkRenderer.h"
#include "DevConsole.h"
#include "InputMapper.h"
#include "Inputs.h"
#include "LoadTaskBlockData.h"
#include "RenderUtils.h"
#include "ShaderLoader.h"
#include "SoaEngine.h"
#include "SoAState.h"

#ifdef DEBUG
#define HORIZONTAL_CHUNKS 26
#define VERTICAL_CHUNKS 4
#else

#define HORIZONTAL_CHUNKS 26
#define VERTICAL_CHUNKS 20
#endif

TestBiomeScreen::TestBiomeScreen(const App* app, CommonState* state) :
IAppScreen<App>(app),
m_commonState(state),
m_soaState(m_commonState->state),
m_blockArrayRecycler(1000) {

}

i32 TestBiomeScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestBiomeScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestBiomeScreen::build() {

}

void TestBiomeScreen::destroy(const vui::GameTime& gameTime VORB_UNUSED) {

}

void TestBiomeScreen::onEntry(const vui::GameTime& gameTime VORB_UNUSED) {
    // Init spritebatch and font
    m_sb.init();
    m_font.init("Fonts/orbitron_bold-webfont.ttf", 32);
    // Init game state
    SoaEngine::initState(m_commonState->state);
    // Init access
    m_accessor.init(&m_allocator);
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

    // Load test planet
    PlanetGenLoader planetLoader;
    m_iom.setSearchDirectory("StarSystems/Trinity/");
    planetLoader.init(&m_iom);
    m_genData = planetLoader.loadPlanetGenData("Moons/Aldrin/terrain_gen.yml");
    if (m_genData->terrainColorPixels.data) {
        m_soaState->clientState.blockTextures->setColorMap("biome", &m_genData->terrainColorPixels);
    }

    // Load blocks
    LoadTaskBlockData blockLoader(&m_soaState->blocks,
                                  &m_soaState->clientState.blockTextureLoader,
                                  &m_commonState->loadContext);
    blockLoader.load();

    // Uploads all the needed textures
    m_soaState->clientState.blockTextures->update();
    
    m_genData->radius = 4500.0;
    
    // Set blocks
    SoaEngine::initVoxelGen(m_genData, m_soaState->blocks);
    
    m_chunkGenerator.init(m_genData);

    initHeightData();
    initChunks();

    printf("Generating Meshes...\n");
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

    // Initialize dev console
    vui::GameWindow* window = m_commonState->window;
    m_devConsoleView.init(&DevConsole::getInstance(), 5,
                          f32v2(20.0f, window->getHeight() - 400.0f),
                          window->getWidth() - 40.0f);

    // Set GL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearDepth(1.0);
    printf("Done.");
}

void TestBiomeScreen::onExit(const vui::GameTime& gameTime VORB_UNUSED) {
    for (auto& cv : m_chunks) {
        m_mesher.freeChunkMesh(cv.chunkMesh);
    }
    m_hdrTarget.dispose();
    m_swapChain.dispose();

    m_devConsoleView.dispose();
}

void TestBiomeScreen::update(const vui::GameTime& gameTime) {
    f32 speed = 10.0f;
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

void TestBiomeScreen::draw(const vui::GameTime& gameTime VORB_UNUSED) {
    // Bind the FBO
    m_hdrTarget.useGeometry();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Opaque
    m_renderer.beginOpaque(m_soaState->clientState.blockTextures->getAtlasTexture(),
                           f32v3(0.0f, 0.0f, -1.0f), f32v3(1.0f),
                           f32v3(0.3f));
    for (auto& vc : m_chunks) {
        if (m_camera.getFrustum().sphereInFrustum(f32v3(vc.chunkMesh->position + f64v3(CHUNK_WIDTH / 2) - m_camera.getPosition()), CHUNK_DIAGONAL_LENGTH)) {
            vc.inFrustum = true;
            m_renderer.drawOpaque(vc.chunkMesh, m_camera.getPosition(), m_camera.getViewProjectionMatrix());
        } else {
            vc.inFrustum = false;
        }
    }
    // Cutout
    m_renderer.beginCutout(m_soaState->clientState.blockTextures->getAtlasTexture(),
                           f32v3(0.0f, 0.0f, -1.0f), f32v3(1.0f),
                           f32v3(0.3f));
    for (auto& vc : m_chunks) {
        if (vc.inFrustum) {
            m_renderer.drawCutout(vc.chunkMesh, m_camera.getPosition(), m_camera.getViewProjectionMatrix());
        }
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

    // Draw dev console
    m_devConsoleView.update(0.01f);
    m_devConsoleView.render(m_game->getWindow().getViewportDims());
}

void TestBiomeScreen::initHeightData() {
    printf("Generating height data...\n");
    m_heightData.resize(HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS);
    // Init height data
    m_heightGenerator.init(m_genData);
    for (int z = 0; z < HORIZONTAL_CHUNKS; z++) {
        for (int x = 0; x < HORIZONTAL_CHUNKS; x++) {
            auto& hd = m_heightData[z * HORIZONTAL_CHUNKS + x];
            for (int i = 0; i < CHUNK_WIDTH; i++) {
                for (int j = 0; j < CHUNK_WIDTH; j++) {
                    VoxelPosition2D pos;
                    pos.pos.x = x * CHUNK_WIDTH + j + 3000;
                    pos.pos.y = z * CHUNK_WIDTH + i + 3000;
                    PlanetHeightData& data = hd.heightData[i * CHUNK_WIDTH + j];
                    m_heightGenerator.generateHeightData(data, pos);
                    data.temperature = 128;
                    data.humidity = 128;
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
}


void TestBiomeScreen::initChunks() {
    printf("Generating chunks...\n");
    m_chunks.resize(HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS * VERTICAL_CHUNKS);

    // Generate chunk data
    for (size_t i = 0; i < m_chunks.size(); i++) {
        ChunkPosition3D pos;
        i32v3 gridPosition;
        gridPosition.x = i % HORIZONTAL_CHUNKS;
        gridPosition.y = i / (HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS);
        gridPosition.z = (i % (HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS)) / HORIZONTAL_CHUNKS;
        // Center position about origin
        pos.pos.x = gridPosition.x - HORIZONTAL_CHUNKS / 2;
        pos.pos.y = gridPosition.y - VERTICAL_CHUNKS / 4;
        pos.pos.z = gridPosition.z - HORIZONTAL_CHUNKS / 2;
        // Init parameters
        ChunkID id(pos.x, pos.y, pos.z);
        m_chunks[i].chunk = m_accessor.acquire(id);
        m_chunks[i].chunk->init(WorldCubeFace::FACE_TOP);
        m_chunks[i].gridPosition = gridPosition;
        m_chunks[i].chunk->gridData = &m_heightData[i % (HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS)];
        // Generate the chunk
        m_chunkGenerator.generateChunk(m_chunks[i].chunk, m_chunks[i].chunk->gridData->heightData);
        // Decompress to flat array
        m_chunks[i].chunk->blocks.setArrayRecycler(&m_blockArrayRecycler);
        m_chunks[i].chunk->blocks.changeState(vvox::VoxelStorageState::FLAT_ARRAY, m_chunks[i].chunk->dataMutex);
    }

    // Generate flora
    std::vector<FloraNode> lNodes;
    std::vector<FloraNode> wNodes;
    // TODO(Ben): I know this is ugly
    PreciseTimer t1;
    t1.start();
    for (size_t i = 0; i < m_chunks.size(); i++) {
        Chunk* chunk = m_chunks[i].chunk;
        m_floraGenerator.generateChunkFlora(chunk, m_heightData[i % (HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS)].heightData, lNodes, wNodes);
        for (auto& node : wNodes) {
            i32v3 gridPos = m_chunks[i].gridPosition;
            gridPos.x += FloraGenerator::getChunkXOffset(node.chunkOffset);
            gridPos.y += FloraGenerator::getChunkYOffset(node.chunkOffset);
            gridPos.z += FloraGenerator::getChunkZOffset(node.chunkOffset);
            if (gridPos.x >= 0 && gridPos.y >= 0 && gridPos.z >= 0
                && gridPos.x < HORIZONTAL_CHUNKS && gridPos.y < VERTICAL_CHUNKS && gridPos.z < HORIZONTAL_CHUNKS) {
                Chunk* chunk = m_chunks[gridPos.x + gridPos.y * HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS + gridPos.z * HORIZONTAL_CHUNKS].chunk;
                chunk->blocks.set(node.blockIndex, node.blockID);
            }
        }
        for (auto& node : lNodes) {
            i32v3 gridPos = m_chunks[i].gridPosition;
            gridPos.x += FloraGenerator::getChunkXOffset(node.chunkOffset);
            gridPos.y += FloraGenerator::getChunkYOffset(node.chunkOffset);
            gridPos.z += FloraGenerator::getChunkZOffset(node.chunkOffset);
            if (gridPos.x >= 0 && gridPos.y >= 0 && gridPos.z >= 0
                && gridPos.x < HORIZONTAL_CHUNKS && gridPos.y < VERTICAL_CHUNKS && gridPos.z < HORIZONTAL_CHUNKS) {
                Chunk* chunk = m_chunks[gridPos.x + gridPos.y * HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS + gridPos.z * HORIZONTAL_CHUNKS].chunk;
                if (chunk->blocks.get(node.blockIndex) == 0) {
                    chunk->blocks.set(node.blockIndex, node.blockID);
                }
            }
        }
        std::vector<ui16>().swap(chunk->floraToGenerate);
        lNodes.clear();
        wNodes.clear();
    }
    printf("Tree Gen Time %lf\n", t1.stop());

#define GET_INDEX(x, y, z) ((x) + (y) * HORIZONTAL_CHUNKS * HORIZONTAL_CHUNKS + (z) * HORIZONTAL_CHUNKS)
    
    // Set neighbor pointers
    for (int y = 0; y < VERTICAL_CHUNKS; y++) {
        for (int z = 0; z < HORIZONTAL_CHUNKS; z++) {
            for (int x = 0; x < HORIZONTAL_CHUNKS; x++) {
                Chunk* chunk = m_chunks[GET_INDEX(x, y, z)].chunk;
                // TODO(Ben): Release these too.
                if (x > 0) {
                    chunk->neighbor.left = m_chunks[GET_INDEX(x - 1, y, z)].chunk.acquire();
                }
                if (x < HORIZONTAL_CHUNKS - 1) {
                    chunk->neighbor.right = m_chunks[GET_INDEX(x + 1, y, z)].chunk.acquire();
                }
                if (y > 0) {
                    chunk->neighbor.bottom = m_chunks[GET_INDEX(x, y - 1, z)].chunk.acquire();
                }
                if (y < VERTICAL_CHUNKS - 1) {
                    chunk->neighbor.top = m_chunks[GET_INDEX(x, y + 1, z)].chunk.acquire();
                }
                if (z > 0) {
                    chunk->neighbor.back = m_chunks[GET_INDEX(x, y, z - 1)].chunk.acquire();
                }
                if (z < HORIZONTAL_CHUNKS - 1) {
                    chunk->neighbor.front = m_chunks[GET_INDEX(x, y, z + 1)].chunk.acquire();
                }
            }
        }
    }
}

void TestBiomeScreen::initInput() {

    m_inputMapper = new InputMapper;
    initInputs(m_inputMapper);

    m_mouseButtons[0] = false;
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onMotion, [&](Sender s VORB_UNUSED, const vui::MouseMotionEvent& e) {
        if (m_mouseButtons[0]) {
            m_camera.rotateFromMouseAbsoluteUp(-e.dx, -e.dy, 0.01f, true);
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::mouse.onButtonDown, [&](Sender s VORB_UNUSED, const vui::MouseButtonEvent& e) {
        if (e.button == vui::MouseButton::LEFT) m_mouseButtons[0] = !m_mouseButtons[0];
        if (m_mouseButtons[0]) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        else {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&](Sender s VORB_UNUSED, const vui::KeyEvent& e) {
        PlanetGenLoader planetLoader;
        switch (e.keyCode) {
            case VKEY_ESCAPE:
                exit(0);
                break;
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
            case VKEY_F12:
                // Reload world
                delete m_genData;
                planetLoader.init(&m_iom);
                m_genData = planetLoader.loadPlanetGenData("Planets/Aldrin/terrain_gen.yml");
                m_genData->radius = 4500.0;

                // Set blocks
                SoaEngine::initVoxelGen(m_genData, m_soaState->blocks);

                m_chunkGenerator.init(m_genData);

                for (auto& cv : m_chunks) {
                    m_mesher.freeChunkMesh(cv.chunkMesh);
                    cv.chunk.release();
                }

                initHeightData();
                initChunks();

                printf("Generating Meshes...\n");
                // Create all chunk meshes
                m_mesher.init(&m_soaState->blocks);
                for (auto& cv : m_chunks) {
                    cv.chunkMesh = m_mesher.easyCreateChunkMesh(cv.chunk, MeshTaskType::DEFAULT);
                }
                break;
        }
    });
    m_hooks.addAutoHook(vui::InputDispatcher::key.onKeyUp, [&](Sender s VORB_UNUSED, const vui::KeyEvent& e) {
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

    // Dev console
    m_hooks.addAutoHook(m_inputMapper->get(INPUT_DEV_CONSOLE).downEvent, [](Sender s VORB_UNUSED, ui32 a VORB_UNUSED) {
        DevConsole::getInstance().toggleFocus();
    });

    m_inputMapper->startInput();
}
