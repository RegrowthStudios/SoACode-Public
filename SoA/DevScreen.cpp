#include "stdafx.h"
#include "DevScreen.h"

#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>
#include <Vorb/colors.h>
#include <Vorb/ui/KeyStrings.h>

#define DEV_SCREEN_FONT "Fonts/orbitron_bold-webfont.ttf"
#define DEV_SCREEN_FONT_SIZE 32

const cString TITLE = "Dev Screen";
const color4& FONT_COLOR = color::AliceBlue;

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
void DevScreen::destroy(const vui::GameTime&) {
    // Empty
}

void DevScreen::onEntry(const vui::GameTime&) {
    m_delegatePool.addAutoHook(vui::InputDispatcher::key.onKeyDown, [&] (Sender, const vui::KeyEvent& e) {
        auto kvp = m_screenMapping.find((VirtualKey)e.keyCode);
        if (kvp == m_screenMapping.end()) return;
        m_nextScreen = kvp->second;
    });

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClearDepth(1.0);

    m_nextScreen = nullptr;
    deferCounter = 1;

    m_sb = new vg::SpriteBatch(true, true);
    m_font = new vg::SpriteFont();
    m_font->init(DEV_SCREEN_FONT, DEV_SCREEN_FONT_SIZE);
}
void DevScreen::onExit(const vui::GameTime&) {
    m_delegatePool.dispose();
    m_sb->dispose();
    delete m_sb;
    m_sb = nullptr;

    m_font->dispose();
    delete m_font;
    m_font = nullptr;
}

void DevScreen::update(const vui::GameTime&) {
    if (m_nextScreen) {
        if (deferCounter) {
            --deferCounter;
        } else {
            m_state = vui::ScreenState::CHANGE_NEXT;
        }
    }
}

void DevScreen::draw(const vui::GameTime&) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const vui::GameWindow* w = &m_game->getWindow();

    m_sb->begin();

    f32v2 pos(90.0f, 300.0f);
    f32 posInc = 35.0f;

    if (m_nextScreen) {
        // Draw loading message
        m_sb->drawString(m_font, "Loading... please wait.", f32v2(w->getWidth() * 0.5f, w->getHeight() * 0.5f), f32v2(1.0f), FONT_COLOR, vg::TextAlign::CENTER);
    } else {
        // Draw title
        m_sb->drawString(m_font, TITLE, f32v2((w->getWidth() - m_font->measure(TITLE).x * 1.5) / 2.0, 50.0f),
                         f32v2(1.5f), FONT_COLOR);
        // Draw strings
        m_sb->drawString(m_font, "* Press one of the following keys to enter a screen:", pos, f32v2(1.0f), FONT_COLOR);
   
        pos.y += posInc * 2.0f;
        for (auto& it : m_screenMapping) {
            m_sb->drawString(m_font, m_screenNames[it.first].c_str(), pos, f32v2(1.0f), FONT_COLOR);
            pos.y += posInc;
        }
    }
    m_sb->end();
    m_sb->render(f32v2(w->getWidth(), w->getHeight()));
}

void DevScreen::addScreen(VirtualKey vKey, vui::IGameScreen* s, const nString& name) {
    m_screenMapping[vKey] = s;
    m_screenNames[vKey] = nString(VirtualKeyStrings[vKey]) + ": " + name;
}
