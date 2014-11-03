#include "stdafx.h"
#include "DevConsoleView.h"

#include <algorithm>
#include <iterator>

#include "DevConsole.h"
#include "GLStates.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"

DevConsoleView::DevConsoleView() :
_batch(nullptr),
_font(nullptr),
_console(nullptr),
_blinkTimeRemaining(DEV_CONSOLE_MARKER_BLINK_DELAY),
_isViewModified(false),
_linesToRender(2),
_renderRing(2) {}
DevConsoleView::~DevConsoleView() {
    dispose();
}

void DevConsoleView::init(DevConsole* console, i32 linesToRender, vg::GLProgramManager* glProgramManager) {
    _renderRing.set_capacity(linesToRender);
    _renderRing.clear();

    _console = console;
    _fHook = [] (void* meta, const nString& str) {
        ((DevConsoleView*)meta)->onNewCommand(str);
    };
    _console->addListener(_fHook, this);
    _isViewModified = true;

    _batch = new SpriteBatch(true, true);

    _font = new SpriteFont("Fonts\\chintzy.ttf", 32);
}
void DevConsoleView::dispose() {
    if (_batch) {
        _batch->dispose();
        _batch = nullptr;
    }
    if (_font) {
        _font->dispose();
        _font = nullptr;
    }
    if (_console) {
        if (_fHook) {
            _console->removeListener(_fHook);
            _fHook = nullptr;
        }
        _console = nullptr;
    }
}

void DevConsoleView::onEvent(const SDL_Event& e) {
    // TODO: Input
}
void DevConsoleView::update(const f32& dt) {
    // Blinking Logic
    _blinkTimeRemaining -= dt;
    if (_blinkTimeRemaining < 0) {
        _blinkTimeRemaining = DEV_CONSOLE_MARKER_BLINK_DELAY;
        _isViewModified = true;
    }

    if (_isViewModified) redrawBatch();
}

void DevConsoleView::render(const f32v2& position, const f32v2& screenSize) {
    // Check For A Batch
    if (!_batch) return;

    // Translation Matrix To Put Top-Left Cornert To Desired Position
    f32m4 mTranslate(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        position.x, position.y, 0, 1
        );
    _batch->renderBatch(mTranslate, screenSize, &SamplerState::POINT_WRAP, &DepthState::NONE, &RasterizerState::CULL_NONE);
}

void DevConsoleView::onNewCommand(const nString& str) {
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, '\n')) {
        _renderRing.push_back(item);
    }

    _isViewModified = true;
}

void DevConsoleView::redrawBatch() {
    if (!_batch || !_font) return;

    _batch->begin();

    // TODO: Draw Background
    f32 textHeight = (f32)_font->getFontHeight();

    // Draw Command Lines
    for (i32 i = 0; i < _renderRing.size(); i++) {
        const cString cStr = _renderRing[i].c_str();
        if (cStr) {
            _batch->drawString(_font, cStr,
                f32v2(10, textHeight * i + 10),
                f32v2(1),
                ColorRGBA8(0, 255, 0, 255),
                0.9f
                );
        }
    }

    // TODO: Draw Input

    _batch->end(SpriteSortMode::BACK_TO_FRONT);
}
