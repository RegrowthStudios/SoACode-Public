#include "stdafx.h"
#include "TestConsoleScreen.h"

#include <InputDispatcher.h>

i32 TestConsoleScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 TestConsoleScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestConsoleScreen::build() {
    // Empty
}
void TestConsoleScreen::destroy(const GameTime& gameTime) {
    // Empty
}

void TestConsoleScreen::onEntry(const GameTime& gameTime) {
    m_delegatePool.addAutoHook(&m_console.onStream[DEV_CONSOLE_STREAM_OUT], [&] (void* sender, const cString s) {
        printf("Out:   %s\n", s);
    });
    m_delegatePool.addAutoHook(&m_console.onStream[DEV_CONSOLE_STREAM_ERR], [&] (void* sender, const cString s) {
        printf("Err:   %s\n", s);
    });
    m_delegatePool.addAutoHook(&m_text.onTextChange, [&] (void* sender, const cString s) {
        printf("\rInput: %s  ", s);
    });
    m_delegatePool.addAutoHook(&m_text.onTextEntry, [&] (void* sender, const cString s) {
        printf("\rComm:  %s\n", s);
        m_console.invokeCommand(s);
    });
    m_text.start();
    printf("Welcome to Lua REPL\nInput: ");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
}
void TestConsoleScreen::onExit(const GameTime& gameTime) {
    m_text.stop();
    m_delegatePool.dispose();
}

void TestConsoleScreen::onEvent(const SDL_Event& e) {
    // Empty
}
void TestConsoleScreen::update(const GameTime& gameTime) {
    // Empty
}
void TestConsoleScreen::draw(const GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
