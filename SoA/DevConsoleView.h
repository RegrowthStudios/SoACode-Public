#pragma once

#include <SDL/SDL.h>
#include <Vorb/RingBuffer.hpp>
#include <Vorb/VorbPreDecl.inl>
#include <Vorb/colors.h>

class DevConsole;

DECL_VG(class SpriteBatch;
        class SpriteFont)

typedef vorb::ring_buffer<nString> StringRing;

const f32 DEV_CONSOLE_MARKER_BLINK_DELAY = 0.85f;
const int START_LINES_TO_RENDER = 2;

class DevConsoleView {
public:
    DevConsoleView();
    ~DevConsoleView();

    void init(DevConsole* console, i32 linesToRender, const f32v2& position, const f32v2& dimensions);
   

    void dispose();

    void update(const f32& dt);

    void render(const f32v2& position, const f32v2& screenSize);

    /************************************************************************/
    /* Setters                                                              */
    /************************************************************************/
    void setBackColor(const color4& color);
    void setFontColor(const color4& color);
    void setPosition(const f32v2& pos);
    void setDimensions(const f32v2& dims);

    /************************************************************************/
    /* Getters                                                              */
    /************************************************************************/
    const color4& getBackColor() const { return m_backColor; }
    const color4& getFontColor() const { return m_fontColor; }
    const f32v2& getPosition() const { return m_position; }
    const f32v2& getDimensions() const { return m_dimensions; }
private:
    void onNewCommand(const nString& str);
    void redrawBatch();

    DevConsole* m_console = nullptr;
    void(*_fHook) (void*, const nString&);

    vg::SpriteBatch* m_batch = nullptr;
    vg::SpriteFont* m_font = nullptr;
    bool m_isViewModified = false;
    nString m_currentLine = "";

    f32v2 m_position = f32v2(0.0f);
    f32v2 m_dimensions = f32v2(0.0f);
    color4 m_backColor = color4(0, 0, 0, 128);
    color4 m_fontColor = color::LightGreen;

    i32 m_linesToRender = START_LINES_TO_RENDER;
    StringRing m_renderRing;
    f32 m_blinkTimeRemaining = DEV_CONSOLE_MARKER_BLINK_DELAY;
};
