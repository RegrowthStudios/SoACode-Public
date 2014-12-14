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
    m_delegatePool.addAutoHook(&vui::InputDispatcher::key.onText, [&] (void* sender, const vui::TextEvent& e) {
        m_command += nString(e.text);
        printf("\rCC: %s", m_command.c_str());
    });
    m_delegatePool.addAutoHook(&vui::InputDispatcher::key.onKeyDown, [&] (void* sender, const vui::KeyEvent& e) {
        switch (e.keyCode) {
        case SDLK_RETURN:
            printf("\rCM: %s\n", m_command.c_str());
            m_console.invokeCommand(m_command);
            m_command = "";
            break;
        case SDLK_BACKSPACE:
            if (m_command.length() > 0) {
                m_command = m_command.substr(0, m_command.length() - 1);
                printf("\rCC: %s ", m_command.c_str());
            }
            break;
        default:
            break;
        }
    });
    m_delegatePool.addAutoHook(&m_console.onStream[DEV_CONSOLE_STREAM_OUT], [&] (void* sender, const cString s) {
        printf("OO: %s\n", s);
    });
    m_delegatePool.addAutoHook(&m_console.onStream[DEV_CONSOLE_STREAM_ERR], [&] (void* sender, const cString s) {
        printf("OE: %s\n", s);
    });
    puts("Welcome to Lua REPL");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
}
void TestConsoleScreen::onExit(const GameTime& gameTime) {
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
