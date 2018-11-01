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

    m_viewport.init("TestUIScreen", f32v4(0.0f, 0.0f, window->getWidth(), window->getHeight()), &m_font, &m_sb);

    // TODO(Matthew): Create elements and attach to viewport.

    

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
