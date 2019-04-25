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

    m_panels[0].setZIndex(1);
    m_panels[1].setZIndex(2);
    m_panels[2].setZIndex(3);
    m_panels[3].setZIndex(4);
    m_panels[4].setZIndex(5);

    m_panels[0].setAutoScroll(true);
    m_panels[1].setAutoScroll(true);
    m_panels[2].setAutoScroll(true);
    m_panels[3].setAutoScroll(true);
    m_panels[4].setAutoScroll(true);

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
    m_checkBox.setPadding(f32v4(10.0f, 5.0f, 10.0f, 5.0f));
    m_checkBox.setText("Hello, World!");
    m_checkBox.setTextScale(f32v2(0.65f));
    m_checkBox.setTextAlign(vg::TextAlign::CENTER);
    m_checkBox.setClipping(vui::Clipping{ vui::ClippingState::HIDDEN, vui::ClippingState::HIDDEN, vui::ClippingState::HIDDEN, vui::ClippingState::HIDDEN });

    m_panels[0].addWidget(&m_checkBox);

    m_label.init("Label", f32v4(130.0f, 130.0f, 120.0f, 30.0f));
    m_label.setPadding(f32v4(10.0f, 5.0f, 10.0f, 5.0f));
    m_label.setText("Wooooo!");
    m_label.setTextScale(f32v2(0.65f));
    m_label.setTextAlign(vg::TextAlign::CENTER);

    m_panels[1].addWidget(&m_label);

    m_button.init("Button", f32v4(60.0f, 130.0f, 120.0f, 30.0f));
    m_button.setPadding(f32v4(10.0f, 5.0f, 10.0f, 5.0f));
    m_button.setText("Click Me!");
    m_button.setTextScale(f32v2(0.65f));
    m_button.setTextAlign(vg::TextAlign::CENTER);

    m_panels[2].addWidget(&m_button);

    m_comboBox.init("ComboBox", f32v4(60.0f, 130.0f, 170.0f, 30.0f));

    m_comboBox.addItem("This is One.");
    m_comboBox.addItem("This is Two.");
    m_comboBox.addItem("This is Three.");
    m_comboBox.addItem("This is Four.");

    // TODO(Matthew): Padding not supported by combobox yet, I think - too distracted to check, just occurred to me.
    // m_comboBox.setPadding(f32v4(10.0f, 5.0f, 10.0f, 5.0f));
    m_comboBox.setText("Click Me!");
    m_comboBox.setTextScale(f32v2(0.65f));
    m_comboBox.setTextAlign(vg::TextAlign::CENTER);
    m_comboBox.setBackColor(color::Aquamarine);
    m_comboBox.setBackHoverColor(color::Azure);
    m_comboBox.setZIndex(2);

    m_comboBox.setMaxDropHeight(90.0f);

    m_comboBox.selectItem(0);

    m_panels[4].addWidget(&m_comboBox);

    m_widgetList.init("WidgetList", f32v4(50.0f, 50.0f, 200.0f, 200.0f));
    m_widgetList.setBackColor(color::Bisque);
    m_widgetList.setBackHoverColor(color::Crimson);
    m_widgetList.setAutoScroll(true);
    m_widgetList.setSpacing(0.0f);
    m_widgetList.setMaxHeight(200.0f);

    size_t i = 0;
    for (auto& button : m_listButtons) {
        button.init("ListButton" + std::to_string(i++), f32v4(0.0f, 0.0f, 200.0f, 50.0f));
        button.setBackColor(color4(40.0f * (f32)i, 20.0f, 20.0f));
        button.setBackHoverColor(color4(20.0f, 40.0f * (f32)i, 20.0f));
        button.setText("ListButton" + std::to_string(i));
        button.setTextScale(f32v2(0.65f));
        button.setTextAlign(vg::TextAlign::CENTER);

        m_widgetList.addItem(&button);
    }

    m_panels[3].addWidget(&m_widgetList);

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
