#include "stdafx.h"
#include "TestConnectedTextureScreen.h"

i32 TestConnectedTextureScreen::getNextScreen() const {

}

i32 TestConnectedTextureScreen::getPreviousScreen() const {

}

void TestConnectedTextureScreen::build() {

}

void TestConnectedTextureScreen::destroy(const vui::GameTime& gameTime) {

}

void TestConnectedTextureScreen::onEntry(const vui::GameTime& gameTime) {
    m_chunks.emplace_back(new Chunk);
}

void TestConnectedTextureScreen::onExit(const vui::GameTime& gameTime) {

}

void TestConnectedTextureScreen::update(const vui::GameTime& gameTime) {

    
}

void TestConnectedTextureScreen::draw(const vui::GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


}
