#pragma once
#include <Vorb/RingBuffer.hpp>
#include <SDL/SDL.h>
#include <Vorb/VorbPreDecl.inl>

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

    void init(DevConsole* console, i32 linesToRender);
    void dispose();

    void update(const f32& dt);

    void render(const f32v2& position, const f32v2& screenSize);
private:
    void onNewCommand(const nString& str);
    void redrawBatch();

    DevConsole* m_console = nullptr;
    void(*_fHook) (void*, const nString&);

    vg::SpriteBatch* m_batch = nullptr;
    vg::SpriteFont* m_font = nullptr;
    bool m_isViewModified = false;
    nString m_currentLine = "";

    i32 m_linesToRender = START_LINES_TO_RENDER;
    StringRing m_renderRing;
    f32 m_blinkTimeRemaining = DEV_CONSOLE_MARKER_BLINK_DELAY;
};
