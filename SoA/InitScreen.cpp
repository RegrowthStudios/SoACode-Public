#include "stdafx.h"
#include "InitScreen.h"

#include <Vorb/colors.h>
#include <Vorb/graphics/GLStates.h>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>

#include "App.h"
#include "LoadScreen.h"
#include "GameManager.h"

#define INIT_SCREEN_FONT "Fonts/orbitron_bold-webfont.ttf"
#define INIT_SCREEN_FONT_SIZE 32
#define COLOR_FAILURE Red
#define COLOR_SUCCESS LimeGreen

i32 InitScreen::getNextScreen() const {
    return _app->scrLoad->getIndex();
}
i32 InitScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void InitScreen::build() {
    // Empty
}
void InitScreen::destroy(const vui::GameTime& gameTime) {
    // Empty
}

void InitScreen::onEntry(const vui::GameTime& gameTime) {
    buildSpriteResources();

    checkRequirements();

    // Background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
}
void InitScreen::onExit(const vui::GameTime& gameTime) {
    destroySpriteResources();
}

void InitScreen::update(const vui::GameTime& gameTime) {
    // Immediatly move to next state
    _state = vui::ScreenState::CHANGE_NEXT;
}
void InitScreen::draw(const vui::GameTime& gameTime) {
    const vui::GameWindow* w = &_game->getWindow();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _sb->renderBatch(f32v2(w->getWidth(), w->getHeight()), &vg::SamplerState::LINEAR_WRAP, &vg::DepthState::FULL, &vg::RasterizerState::CULL_NONE);
}

void InitScreen::buildSpriteResources() {
    _sb = new vg::SpriteBatch(true, true);
    _font = new vg::SpriteFont();
    _font->init(INIT_SCREEN_FONT, INIT_SCREEN_FONT_SIZE);
}
void InitScreen::destroySpriteResources() {
    _sb->dispose();
    delete _sb;
    _sb = nullptr;

    _font->dispose();
    delete _font;
    _font = nullptr;
}

void InitScreen::checkRequirements() {
    static const f32 textSize = 30.0f;
    static const f32 textOffset = 5.0f;

    _canContinue = true;

    const vui::GameWindow* w = &_game->getWindow();

    f32v2 pos(0, 0);
    f32v2 rectSize(w->getWidth(), textSize + textOffset * 2.0f);
    f32v2 textOff(textOffset, textOffset);

    // Check If Application Can Proceed
#define INIT_BRANCH(MESSAGE) { \
    _sb->draw(0, pos, rectSize, color::COLOR_FAILURE, 0.5f); \
    _sb->drawString(_font, MESSAGE, pos + textOff, textSize, 1.0f, color::White, 0.0f); \
    _canContinue = false; \
    } else { \
        _sb->draw(0, pos, rectSize, color::COLOR_SUCCESS, 0.5f); \
        _sb->drawString(_font, MESSAGE, pos + textOff, textSize, 1.0f, color::White, 0.0f); \
    } \
    pos.y += rectSize.y;

    const vg::GraphicsDeviceProperties gdProps = vg::GraphicsDevice::getCurrent()->getProperties();
    _sb->begin();
    if (gdProps.glVersionMajor < 3) INIT_BRANCH("OpenGL Version");
    if (!GLEW_VERSION_2_1) INIT_BRANCH("GLEW 2.1");
    if (gdProps.maxTextureUnits < 8) INIT_BRANCH("Texture Units");

    // Inform User What Will Happen
    pos.y += textSize * 0.5f;
    if (_canContinue) {
        _sb->draw(0, pos, rectSize, color::COLOR_SUCCESS, 0.5f);
        _sb->drawString(_font, "Application Will Proceed", pos + textOff, textSize, 1.0f, color::White, 0.0f);
    } else {
        _sb->draw(0, pos, rectSize, color::COLOR_FAILURE, 0.5f);
        _sb->drawString(_font, "Application Will Now Exit", pos + textOff, textSize, 1.0f, color::White, 0.0f);
    }
    _sb->drawString(_font, "Press Any Key To Continue", f32v2(10.0f, w->getHeight() - 30.0f), 24.0f, 1.0f, color::LightGray, 0.0f);
    _sb->end(vg::SpriteSortMode::TEXTURE);

#ifdef DEBUG
    printf("System Met Minimum Requirements\n");
#endif // DEBUG
}
