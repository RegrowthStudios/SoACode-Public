#include "stdafx.h"
#include "TestConnectedTextureScreen.h"

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

    // Create Chunks
    Chunk* chunk = new Chunk;
    chunk->initAndFillEmpty(0, ChunkPosition3D());
    chunk->blocks.set(CHUNK_SIZE / 2, 43);
    m_chunks.emplace_back(chunk);

    // Create all chunk meshes
    m_mesher.init(&m_soaState->blocks);
    for (auto& chunk : m_chunks) {
        m_meshes.emplace_back(m_mesher.easyCreateChunkMesh(chunk, MeshTaskType::DEFAULT));
    }

    // Set camera position
    m_camera.init(m_commonState->window->getAspectRatio());
    m_camera.setPosition(f64v3(16.0, 16.0, 30.0));
    m_camera.setDirection(f32v3(0.0f, 0.0f, -1.0f));
    m_camera.setRight(f32v3(1.0f, 0.0f, 0.0f));
    m_camera.setDirection(f32v3(0.0f, 1.0f, 0.0f));
}

void TestConnectedTextureScreen::onExit(const vui::GameTime& gameTime) {
    for (auto& cm : m_meshes) {
        delete cm;
    }
}

void TestConnectedTextureScreen::update(const vui::GameTime& gameTime) {
    m_camera.update();
}

void TestConnectedTextureScreen::draw(const vui::GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_renderer.beginOpaque(m_soaState->blockTextures->getAtlasTexture(),
                           f32v3(0.0f, 0.0f, -1.0f), f32v3(1.0f),
                           f32v3(0.3f));
    m_renderer.drawOpaque(m_meshes[m_activeChunk], m_camera.getPosition(), m_camera.getViewProjectionMatrix());
    m_renderer.end();
}
