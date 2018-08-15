#include "stdafx.h"
#include "TestConsoleScreen.h"

#include <Vorb/ui/InputDispatcher.h>

i32 TestConsoleScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 TestConsoleScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestConsoleScreen::build() {
    // Empty
}
void TestConsoleScreen::destroy(const vui::GameTime& gameTime VORB_UNUSED) {
    // Empty
}

void TestConsoleScreen::onEntry(const vui::GameTime& gameTime VORB_UNUSED) {
#ifdef VORB_LUA
    m_delegatePool.addAutoHook(m_console.onStream[DEV_CONSOLE_STREAM_OUT], [&] (Sender sender VORB_UNUSED, const cString s) {
        printf("Out:   %s\n", s);
    });
    m_delegatePool.addAutoHook(m_console.onStream[DEV_CONSOLE_STREAM_ERR], [&] (Sender sender VORB_UNUSED, const cString s) {
        printf("Err:   %s\n", s);
    });
    m_delegatePool.addAutoHook(m_text.onTextChange, [&] (Sender sender VORB_UNUSED, const cString s) {
        printf("\rInput: %s  ", s);
    });
    m_delegatePool.addAutoHook(m_text.onTextEntry, [&] (Sender sender VORB_UNUSED, const cString s) {
        printf("\rComm:  %s\n", s);
        m_console.invokeCommand(s);
    });
    m_text.start();
    printf("Welcome to Lua REPL\nInput: ");
#endif//VORB_LUA
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0);
}
void TestConsoleScreen::onExit(const vui::GameTime& gameTime VORB_UNUSED) {
    m_text.stop();
    m_delegatePool.dispose();
}

void TestConsoleScreen::update(const vui::GameTime& gameTime VORB_UNUSED) {
    // Empty
}
void TestConsoleScreen::draw(const vui::GameTime& gameTime VORB_UNUSED) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
