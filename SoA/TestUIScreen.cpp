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

    m_panels[0].init("panel1", f32v4(0.0f));
    m_panels[1].init("panel2", f32v4(0.0f));
    m_panels[2].init("panel3", f32v4(0.0f));
    m_panels[3].init("panel4", f32v4(0.0f));
    m_panels[4].init("panel5", f32v4(0.0f));

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

    m_checkBox.init("CheckBox", f32v4(30.0f, 30.0f, 150.0f, 30.0f));
    m_checkBox.setRawPadding(f32v4(10.0f, 5.0f, 10.0f, 5.0f));
    m_checkBox.setText("Hello, World!");
    m_checkBox.setTextScale(f32v2(0.65f));
    m_checkBox.setTextAlign(vg::TextAlign::CENTER);
    m_checkBox.setClipping({ vui::ClippingState::HIDDEN, vui::ClippingState::HIDDEN, vui::ClippingState::HIDDEN, vui::ClippingState::HIDDEN });

    m_panels[0].addWidget(&m_checkBox);



    m_viewport.addWidget(&m_panels[0]);
    m_viewport.addWidget(&m_panels[1]);
    m_viewport.addWidget(&m_panels[2]);
    m_viewport.addWidget(&m_panels[3]);
    m_viewport.addWidget(&m_panels[4]);

    m_viewport.enable();
}

void TestUIScreen::onExit(const vui::GameTime&) {
    m_viewport.dispose();
}

void TestUIScreen::update(const vui::GameTime&) {
    m_viewport.update();
}

void TestUIScreen::draw(const vui::GameTime&) {
    glClear(GL_COLOR_BUFFER_BIT);

    m_viewport.draw();
}
