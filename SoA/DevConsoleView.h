#pragma once
#include <Vorb/RingBuffer.hpp>
#include <SDL/SDL.h>
#include <Vorb/VorbPreDecl.inl>

#include "GLProgramManager.h"

class DevConsole;

DECL_VG(class SpriteBatch;
        class SpriteFont)

typedef vorb::ring_buffer<nString> StringRing;

const f32 DEV_CONSOLE_MARKER_BLINK_DELAY = 0.85f;

class DevConsoleView {
public:
    DevConsoleView();
    ~DevConsoleView();

    void init(DevConsole* console, i32 linesToRender, vg::GLProgramManager* glProgramManager);
    void dispose();

    void onEvent(const SDL_Event& e);
    void update(const f32& dt);

    void render(const f32v2& position, const f32v2& screenSize);
private:
    void onNewCommand(const nString& str);

    void redrawBatch();

    DevConsole* _console;
    void(*_fHook) (void*, const nString&);

    vg::SpriteBatch* _batch;
    vg::SpriteFont* _font;
    bool _isViewModified;

    i32 _linesToRender;
    StringRing _renderRing;
    f32 _blinkTimeRemaining;
};
