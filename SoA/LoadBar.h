#pragma once
class SpriteBatch;
class SpriteFont;

class LoadBarCommonProperties {
public:
    LoadBarCommonProperties(const f32v2& offset, const f32v2& size, const f32& moveSpeed, const f32v2& textOffset, const f32& textSize);
    LoadBarCommonProperties() : LoadBarCommonProperties(f32v2(0), f32v2(0), 0, f32v2(0), 0) {}

    f32v2 offsetDirection;
    f32 offsetLength;
    
    f32v2 size;
    f32v2 textOffset;
    f32 textSize;

    f32 movementSpeed;
};

class LoadBar {
public:
    LoadBar(const LoadBarCommonProperties& commonProps);
    LoadBar() : LoadBar(LoadBarCommonProperties()) {}
    ~LoadBar();

    void expand();
    void retract();

    bool getIsRetracting() const {
        return _moveDirection == -1;
    }
    bool getIsExpanding() const {
        return _moveDirection == 1;
    }
    bool getIsStationary() const {
        return _moveDirection == 0;
    }

    void setCommonProperties(const LoadBarCommonProperties& commonProps);
    void setStartPosition(const f32v2& position);
    void setText(const nString& text);
    void setColor(const ColorRGBA8& colorText, const ColorRGBA8& colorBackground);

    void update(f32 dt);

    void draw(SpriteBatch* sb, SpriteFont* sf, ui32 backTexture, f32 depth);
private:
    nString _text;

    LoadBarCommonProperties _commonProps;

    f32v2 _startPosition;
    i32 _moveDirection;
    f32 _lerpAmount;

    ColorRGBA8 _colorText;
    ColorRGBA8 _colorBackground;
};