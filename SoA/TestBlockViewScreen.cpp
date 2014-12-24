#include "stdafx.h"
#include "TestBlockViewScreen.h"

#include <InputDispatcher.h>
#include <IOManager.h>

#include "BlockLoader.h"

i32 TestBlockView::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 TestBlockView::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestBlockView::build() {
    // Empty
}
void TestBlockView::destroy(const GameTime& gameTime) {
    // Empty
}

void TestBlockView::onEntry(const GameTime& gameTime) {
    m_hooks.addAutoHook(&m_blocks.onBlockAddition, [&] (void* s, ui16 id) {
        printf("Loaded Block: %s = %d\n", m_blocks[id].name.c_str(), id);
    });
    m_hooks.addAutoHook(&vui::InputDispatcher::window.onFile, [&] (void* s, const vui::WindowFileEvent& e) {
        loadBlocks(e.file);
    });

    // Set clear state
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
}
void TestBlockView::onExit(const GameTime& gameTime) {
    // Empty
}

void TestBlockView::onEvent(const SDL_Event& e) {
    // Empty
}
void TestBlockView::update(const GameTime& gameTime) {
    // Empty
}
void TestBlockView::draw(const GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void TestBlockView::loadBlocks(const cString file) {
    IOManager iom;
    BlockLoader::load(&iom, file, &m_blocks);
}
