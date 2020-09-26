#include "stdafx.h"
#include "TestYAMLUIScreen.h"

#include <Vorb/ui/InputDispatcher.h>
#include <Vorb/ui/UILoader.h>

#include "App.h"
#include "InputMapper.h"
#include "Inputs.h"

TestYAMLUIScreen::TestYAMLUIScreen(const App* app, CommonState* state) : IAppScreen<App>(app),
    m_commonState(state)
{
    // Empty
}

i32 TestYAMLUIScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestYAMLUIScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestYAMLUIScreen::build() {
    // Empty
}

void TestYAMLUIScreen::destroy(const vui::GameTime&) {
    // Empty
}

void TestYAMLUIScreen::onEntry(const vui::GameTime&) {
    m_textureCache.init(&m_iom);

    vui::GameWindow* window = m_commonState->window;
    m_viewport = vui::Viewport(window);

    m_sb.init();
    m_font.init("Fonts/orbitron_bold-webfont.ttf", 32);

    m_viewport.init("TestYAMLUIScreen", { 0.0f, 0.0f, { vui::DimensionType::PIXEL, vui::DimensionType::PIXEL } }, { 1.0f, 1.0f, { vui::DimensionType::WINDOW_WIDTH_PERCENTAGE, vui::DimensionType::WINDOW_HEIGHT_PERCENTAGE } }, &m_font, &m_sb);

    vui::UILoader::loadFromYAML(&m_iom, "ui_test.yml", &m_textureCache, &m_viewport);

    m_viewport.enable();
}

void TestYAMLUIScreen::onExit(const vui::GameTime&) {
    m_viewport.dispose();
}

void TestYAMLUIScreen::update(const vui::GameTime&) {
    m_viewport.update();
}

void TestYAMLUIScreen::draw(const vui::GameTime&) {
    glClear(GL_COLOR_BUFFER_BIT);

    m_viewport.draw();
}
