#include "stdafx.h"
#include "DevScreen.h"

#include <Vorb/ui/InputDispatcher.h>

i32 DevScreen::getNextScreen() const {
    if (m_nextScreen) return m_nextScreen->getIndex();
    return SCREEN_INDEX_NO_SCREEN;
}
i32 DevScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void DevScreen::build() {
    // Empty
}
void DevScreen::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void DevScreen::onEntry(const vui::GameTime& gameTime) {
    m_delegatePool.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&] (Sender sender, const vui::KeyEvent& e) {
        auto kvp = m_screenMapping.find((VirtualKey)e.keyCode);
        if (kvp == m_screenMapping.end()) return;
        m_nextScreen = kvp->second;
    });

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClearDepth(1.0);

    m_nextScreen = nullptr;
}
void DevScreen::onExit(const vui::GameTime& gameTime) {
    m_delegatePool.dispose();
}

void DevScreen::update(const vui::GameTime& gameTime) {
    if (m_nextScreen) _state = vui::ScreenState::CHANGE_NEXT;
}
void DevScreen::draw(const vui::GameTime& gameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DevScreen::addScreen(VirtualKey vKey, vui::IGameScreen* s) {
    m_screenMapping[vKey] = s;
}
