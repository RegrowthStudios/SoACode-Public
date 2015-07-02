#include "stdafx.h"
#include "TestConnectedTextureScreen.h"

#include "SoaState.h"
#include "SoaEngine.h"
#include "LoadTaskBlockData.h"
#include "ChunkRenderer.h"

TestConnectedTextureScreen::TestConnectedTextureScreen(const App* app, CommonState* state) :
IAppScreen<App>(app),
m_state(state) {

}

i32 TestConnectedTextureScreen::getNextScreen() const {

}

i32 TestConnectedTextureScreen::getPreviousScreen() const {

}

void TestConnectedTextureScreen::build() {

}

void TestConnectedTextureScreen::destroy(const vui::GameTime& gameTime) {

}

void TestConnectedTextureScreen::onEntry(const vui::GameTime& gameTime) {
    // Init game state
    SoaEngine::initState(m_state->state);

    // Load blocks
    LoadTaskBlockData blockLoader(&m_state->state->blocks,
                                  &m_state->state->blockTextureLoader,
                                  &m_state->loadContext);
    blockLoader.load();
    // Uploads all the needed textures
    m_state->state->blockTextures->update();

    // Create Chunks
    Chunk* chunk = new Chunk;
    chunk->initAndFillEmpty(0, ChunkPosition3D());
    chunk->blocks.set(CHUNK_SIZE / 2, 43);
    m_chunks.emplace_back(chunk);

    // Set camera position
    
}

void TestConnectedTextureScreen::onExit(const vui::GameTime& gameTime) {
    m_camera.init(m_state->window->getAspectRatio());
    m_camera.setPosition(f64v3(0.0, 0.0, -10.0));
}

void TestConnectedTextureScreen::update(const vui::GameTime& gameTime) {
    m_camera.update();
    
}

void TestConnectedTextureScreen::draw(const vui::GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}
