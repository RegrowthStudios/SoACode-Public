#pragma once
#include <Vorb/RingBuffer.hpp>
#include <SDL/SDL.h>
#include <Vorb/VorbPreDecl.inl>

class DevConsole;

DECL_VG(class SpriteBatch;
        class SpriteFont)

typedef vorb::ring_buffer<nString> StringRing;

const f32 DEV_CONSOLE_MARKER_BLINK_DELAY = 0.85f;

class DevConsoleView {
public:
    DevConsoleView();
    ~DevConsoleView();

    void init(DevConsole* console, i32 linesToRender);
    void dispose();

    void onEvent(const SDL_Event& e);
    void update(const f32& dt);

    void render(const f32v2& position, const f32v2& screenSize);
private:
    void onNewCommand(const nString& str);

    void redrawBatch();

    DevConsole* _console;
    void(*_fHook) (void*, const nString&);

    vg::SpriteBatch* m_batch;
    vg::SpriteFont* m_font;
    bool m_isViewModified;

    i32 m_linesToRender;
    StringRing m_renderRing;
    f32 m_blinkTimeRemaining;
};
