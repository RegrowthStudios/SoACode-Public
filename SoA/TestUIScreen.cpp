#include "stdafx.h"
#include "TestUIScreen.h"

#include <Vorb/ui/InputDispatcher.h>

#include "App.h"
#include "InputMapper.h"
#include "Inputs.h"

TestUIScreen::TestUIScreen(const App* app, CommonState* state) : IAppScreen<App>(app),
    m_commonState(state)
{
    // Empty
}

i32 TestUIScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 TestUIScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestUIScreen::build() {
    // Empty
}

void TestUIScreen::destroy(const vui::GameTime&) {
    // Empty
}

void TestUIScreen::onEntry(const vui::GameTime&) {
    vui::GameWindow* window = m_commonState->window;
    m_viewport = vui::Viewport(window);

    m_sb.init();
    m_font.init("Fonts/orbitron_bold-webfont.ttf", 32);

    m_viewport.init("TestUIScreen", { 0.0f, 0.0f, { vui::DimensionType::PIXEL, vui::DimensionType::PIXEL } }, { 1.0f, 1.0f, { vui::DimensionType::WINDOW_WIDTH_PERCENTAGE, vui::DimensionType::WINDOW_HEIGHT_PERCENTAGE } }, &m_font, &m_sb);

    // m_panel[0] = vui::Panel("panel", vui::Length2{ 0.0f, 0.0f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } }, vui::Length2{ 0.5f, 0.5f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } });
    // m_panel[1] = vui::Panel("panel2", vui::Length2{ 0.5f, 0.5f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } }, vui::Length2{ 0.5f, 0.5f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } });

    m_panels[0] = vui::Panel("panel1", vui::Length2{ 0.0f, 0.0f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } }, vui::Length2{ 0.5f, 0.5f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } });
    m_panels[1] = vui::Panel("panel2", vui::Length2{ 0.5f, 0.0f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } }, vui::Length2{ 0.5f, 0.5f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } });
    m_panels[2] = vui::Panel("panel3", vui::Length2{ 0.0f, 0.5f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } }, vui::Length2{ 0.5f, 0.5f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } });
    m_panels[3] = vui::Panel("panel4", vui::Length2{ 0.5f, 0.5f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } }, vui::Length2{ 0.5f, 0.5f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE, vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } });

    // m_panel[0].setColor(color4{ 255, 0, 0 });
    // m_panel[0].setHoverColor(color4{ 0, 255, 0 });
    // m_panel[1].setColor(color4{ 0, 0, 255 });
    // m_panel[1].setHoverColor(color4{ 255, 255, 0 });

    m_panels[0].setColor(color4{ 255, 0, 0 });
    m_panels[0].setHoverColor(color4{ 0, 255, 0 });
    m_panels[1].setColor(color4{ 0, 0, 255 });
    m_panels[1].setHoverColor(color4{ 255, 255, 0 });
    m_panels[2].setColor(color4{ 255, 0, 255 });
    m_panels[2].setHoverColor(color4{ 0, 255, 255 });
    m_panels[3].setColor(color4{ 0, 0, 0 });
    m_panels[3].setHoverColor(color4{ 255, 255, 255 });

    // m_viewport.addWidget(&m_panel[0]);
    // m_viewport.addWidget(&m_panel[1]);

    m_viewport.addWidget(&m_panels[0]);
    m_viewport.addWidget(&m_panels[1]);
    m_viewport.addWidget(&m_panels[2]);
    m_viewport.addWidget(&m_panels[3]);

    m_viewport.enable();
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glClearDepth(1.0);
}

void TestUIScreen::onExit(const vui::GameTime&) {
    m_viewport.dispose();
}

void TestUIScreen::update(const vui::GameTime&) {
    m_viewport.update();
}

void TestUIScreen::draw(const vui::GameTime&) {
    glClear(/*GL_DEPTH_BUFFER_BIT |*/ GL_COLOR_BUFFER_BIT);

    m_viewport.draw();
}
