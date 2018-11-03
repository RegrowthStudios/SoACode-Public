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

    m_panels[0] = vui::Panel("panel1", f32v4(0.0f));
    m_panels[1] = vui::Panel("panel2", f32v4(0.0f));
    m_panels[2] = vui::Panel("panel3", f32v4(0.0f));
    m_panels[3] = vui::Panel("panel4", f32v4(0.0f));
    m_panels[4] = vui::Panel("panel5", f32v4(0.0f));

    m_panels[0].setColor(color4{ 255, 0, 0 });
    m_panels[0].setHoverColor(color4{ 0, 255, 0 });
    m_panels[1].setColor(color4{ 0, 0, 255 });
    m_panels[1].setHoverColor(color4{ 255, 255, 0 });
    m_panels[2].setColor(color4{ 255, 0, 255 });
    m_panels[2].setHoverColor(color4{ 0, 255, 255 });
    m_panels[3].setColor(color4{ 0, 0, 0 });
    m_panels[3].setHoverColor(color4{ 255, 255, 255 });
    m_panels[4].setColor(color4{ 123, 34, 235 });
    m_panels[4].setHoverColor(color4{ 234, 100, 0 });

    m_panels[0].setZIndex(5);
    m_panels[1].setZIndex(4);
    m_panels[2].setZIndex(3);
    m_panels[3].setZIndex(2);
    m_panels[4].setZIndex(1);

    m_panels[0].setDockState(vui::DockState::BOTTOM);
    m_panels[1].setDockState(vui::DockState::LEFT);
    m_panels[2].setDockState(vui::DockState::LEFT);
    m_panels[3].setDockState(vui::DockState::TOP);
    m_panels[4].setDockState(vui::DockState::FILL);

    m_panels[0].setRawDockSize({ 0.25f, { vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } });
    m_panels[1].setRawDockSize({ 0.2f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE } });
    m_panels[2].setRawDockSize({ 0.2f, { vui::DimensionType::VIEWPORT_WIDTH_PERCENTAGE } });
    m_panels[3].setRawDockSize({ 0.25f, { vui::DimensionType::VIEWPORT_HEIGHT_PERCENTAGE } });

    m_viewport.addWidget(&m_panels[0]);
    m_viewport.addWidget(&m_panels[1]);
    m_viewport.addWidget(&m_panels[2]);
    m_viewport.addWidget(&m_panels[3]);
    m_viewport.addWidget(&m_panels[4]);

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
