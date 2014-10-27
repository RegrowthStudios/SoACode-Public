#include "stdafx.h"
#include "LoadBar.h"

#include "SpriteBatch.h"

LoadBarCommonProperties::LoadBarCommonProperties(const f32v2& offset, const f32v2& size, const f32& moveSpeed, const f32v2& textOffset, const f32& textSize) :
offsetDirection(offset),
size(size),
movementSpeed(moveSpeed),
textOffset(textOffset),
textSize(textSize) {
    offsetLength = glm::length(offsetDirection);
    offsetDirection *= (1.0f / offsetLength);
}

LoadBar::LoadBar(const LoadBarCommonProperties& commonProps) :
_startPosition(0),
_commonProps(commonProps),
_moveDirection(0),
_lerpAmount(0.0f),
_colorText(0xff, 0xff, 0xff, 0xff),
_colorBackground(0x00, 0x00, 0x00, 0xff) {
}
LoadBar::~LoadBar() {
}

void LoadBar::expand() {
    _moveDirection = 1;
}
void LoadBar::retract() {
    _moveDirection = -1;
}

void LoadBar::setCommonProperties(const LoadBarCommonProperties& commonProps) {
    _commonProps = commonProps;
}
void LoadBar::setStartPosition(const f32v2& position) {
    _startPosition = position;
}
void LoadBar::setText(const nString& text) {
    _text = text;
}
void LoadBar::setColor(const ColorRGBA8& colorText, const ColorRGBA8& colorBackground) {
    _colorText = colorText;
    _colorBackground = colorBackground;
}

void LoadBar::update(f32 dt) {
    if (_moveDirection == 0) return;

    if (_moveDirection > 0) {
        // Expand
        _lerpAmount += _commonProps.movementSpeed * dt;
        if (_lerpAmount > _commonProps.offsetLength) {
            _lerpAmount = _commonProps.offsetLength;
            _moveDirection = 0;
        }
    } else {
        // Retract
        _lerpAmount -= _commonProps.movementSpeed * dt;
        if (_lerpAmount < 0.0f) {
            _lerpAmount = 0.0f;
            _moveDirection = 0;
        }
    }
}

void LoadBar::draw(SpriteBatch* sb, SpriteFont* sf, ui32 backTexture, f32 depth) {
    f32v2 endPos = _startPosition + (_commonProps.offsetDirection * _lerpAmount);
    sb->draw(backTexture, endPos, _commonProps.size, _colorBackground, depth);
    endPos += _commonProps.textOffset;
    sb->drawString(sf, _text.c_str(), endPos, _commonProps.textSize, 1.0f, _colorText, depth - 0.001f);
}
