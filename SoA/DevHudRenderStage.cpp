#include "stdafx.h"
#include "DevHudRenderStage.h"

#include <Vorb/colors.h>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>

#include "App.h"
#include "global.h"

DevHudRenderStage::DevHudRenderStage(const cString fontPath, i32 fontSize,
                                     const App* app, const f32v2& windowDims) :
    _spriteBatch(new SpriteBatch(true, true)),
    _spriteFont(new SpriteFont(fontPath, fontSize)),
    _mode(DevUiModes::HANDS),
    _app(app),
    _windowDims(windowDims),
    _fontHeight(_spriteFont->getFontHeight()) {
    // Empty
}

DevHudRenderStage::~DevHudRenderStage() {
    delete _spriteBatch;
    delete _spriteFont;
}

void DevHudRenderStage::draw() {

    // Lazily load spritebatch
    if (!_spriteBatch) {
        _spriteBatch = new SpriteBatch(true, true);
        _spriteFont = new SpriteFont("Fonts/orbitron_bold-webfont.ttf", 32);
    }

    // Reset the yOffset
    _yOffset = 0;

    _spriteBatch->begin();

    // Draw crosshair
    if (_mode >= DevUiModes::CROSSHAIR) {
        drawCrosshair();
    }

    // Fps Counters
    if (_mode >= DevUiModes::FPS) {
        drawFps();
    }

    // Items in hands
    if (_mode >= DevUiModes::HANDS) {
        drawHands();
    }

    // Positional text
    if (_mode >= DevUiModes::POSITION) {
        drawPosition();
    }

    _spriteBatch->end();
    // Render to the screen
    _spriteBatch->renderBatch(_windowDims);
}

void DevHudRenderStage::cycleMode(int offset /*= 1*/) {
    
    // The last element in DevUiModes
    const int last = static_cast<int>(DevUiModes::LAST);
    // Treat it as an int when cycling so we can do integer math
    int imode = static_cast<int>(_mode) + offset;
    if (imode < 0) {
        // Negative wrap
        imode = last - ((abs(imode) - 1) % (last + 1));
    } else {
        // Positive wrap
        imode = imode % (last + 1);
    }
    // Cast it back to a DevUiMode
    _mode = static_cast<DevUiModes>(imode);
}

void DevHudRenderStage::drawCrosshair() {
    const f32v2 cSize(26.0f);
 //   _spriteBatch->draw(crosshairTexture.id,
 //                      (_windowDims - cSize) / 2.0f,
 //                      cSize,
 //                      ColorRGBA8(255, 255, 255, 128));
}

void DevHudRenderStage::drawHands() {
    const f32v2 SCALE(0.75f);
    char buffer[256];
    // Left Hand
    //if (_player->leftEquippedItem) {
    //    std::sprintf(buffer, "Left Hand: %s (%d)",
    //                 _player->leftEquippedItem->name.c_str(),
    //                 _player->leftEquippedItem->count);

    //    _spriteBatch->drawString(_spriteFont,
    //                             buffer,
    //                             f32v2(0.0f, _windowDims.y - _fontHeight),
    //                             SCALE,
    //                             color::White);
    //}
    //// Right Hand
    //if (_player->rightEquippedItem) {
    //    std::sprintf(buffer, "Right Hand: %s (%d)",
    //                 _player->rightEquippedItem->name.c_str(),
    //                 _player->rightEquippedItem->count);

    //    _spriteBatch->drawString(_spriteFont,
    //                             buffer,
    //                             f32v2(_windowDims.x - _spriteFont->measure(buffer).x, _windowDims.y - _fontHeight),
    //                             SCALE,
    //                             color::White);
    //}
}

void DevHudRenderStage::drawFps() {
    char buffer[256];
    std::sprintf(buffer, "Render FPS: %.0f", _app->getFps());
    _spriteBatch->drawString(_spriteFont,
                             buffer,
                             f32v2(0.0f, _yOffset),
                             f32v2(1.0f),
                             color::White);
    _yOffset += _fontHeight;

    std::sprintf(buffer, "Physics FPS: %.0f", physicsFps);
    _spriteBatch->drawString(_spriteFont,
                             buffer,
                             f32v2(0.0f, _yOffset),
                             f32v2(1.0f),
                             color::White);
    _yOffset += _fontHeight;
}

void DevHudRenderStage::drawPosition() {
    const f32v2 NUMBER_SCALE(0.75f);
    char buffer[256];
    // Grid position
    _yOffset += _fontHeight;

    _spriteBatch->drawString(_spriteFont,
                             "Grid Position",
                             f32v2(0.0f, _yOffset),
                             f32v2(1.0f),
                             color::White);
    _yOffset += _fontHeight;

   /* std::sprintf(buffer, "X %.2f", _player->headPosition.x);
    _spriteBatch->drawString(_spriteFont,
                             buffer,
                             f32v2(0.0f, _yOffset),
                             NUMBER_SCALE,
                             color::White);
    _yOffset += _fontHeight;

    std::sprintf(buffer, "Y %.2f", _player->headPosition.y);
    _spriteBatch->drawString(_spriteFont,
                             buffer,
                             f32v2(0.0f, _yOffset),
                             NUMBER_SCALE,
                             color::White);
    _yOffset += _fontHeight;

    std::sprintf(buffer, "Z %.2f", _player->headPosition.z);
    _spriteBatch->drawString(_spriteFont,
                             buffer,
                             f32v2(0.0f, _yOffset),
                             NUMBER_SCALE,
                             color::White);
    _yOffset += _fontHeight;*/

    // World position
    _yOffset += _fontHeight;
    _spriteBatch->drawString(_spriteFont,
                             "World Position",
                             f32v2(0.0f, _yOffset),
                             f32v2(1.0f),
                             color::White);
    _yOffset += _fontHeight;

  /*  std::sprintf(buffer, "X %-9.2f", _player->worldPosition.x);
    _spriteBatch->drawString(_spriteFont,
                             buffer,
                             f32v2(0.0f, _yOffset),
                             NUMBER_SCALE,
                             color::White);
    _yOffset += _fontHeight;

    std::sprintf(buffer, "Y %-9.2f", _player->worldPosition.y);
    _spriteBatch->drawString(_spriteFont,
                             buffer,
                             f32v2(0.0f, _yOffset),
                             NUMBER_SCALE,
                             color::White);
    _yOffset += _fontHeight;

    std::sprintf(buffer, "Z %-9.2f", _player->worldPosition.z);
    _spriteBatch->drawString(_spriteFont,
                             buffer,
                             f32v2(0.0f, _yOffset),
                             NUMBER_SCALE,
                             color::White);
    _yOffset += _fontHeight;*/
}
