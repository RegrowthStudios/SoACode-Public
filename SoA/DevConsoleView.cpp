#include "stdafx.h"
#include "DevConsoleView.h"

#include <algorithm>
#include <iterator>

#include <Vorb/graphics/GLStates.h>
#include <Vorb/graphics/SpriteBatch.h>
#include <Vorb/graphics/SpriteFont.h>

#include "DevConsole.h"

DevConsoleView::DevConsoleView() :
    m_renderRing(START_LINES_TO_RENDER) {
}

DevConsoleView::~DevConsoleView() {
    dispose();
}

void DevConsoleView::init(DevConsole* console, i32 linesToRender, const f32v2& position, const f32v2& dimensions) {
    m_position = position;
    m_dimensions = dimensions;

    m_renderRing.resize(linesToRender);
    m_renderRing.clear();

    m_console = console;
    _fHook = [](void* meta, const nString& str) {
        ((DevConsoleView*)meta)->onNewCommand(str);
    };
    m_console->addListener(_fHook, this);
    m_isViewModified = true;

    m_batch = new vg::SpriteBatch();
    m_batch->init();
    m_font = new vg::SpriteFont();
    m_font->init("Fonts/orbitron_black-webfont.ttf", 32);
}

void DevConsoleView::dispose() {
    if (m_batch) {
        m_batch->dispose();
        m_batch = nullptr;
    }
    if (m_font) {
        m_font->dispose();
        m_font = nullptr;
    }
    if (m_console) {
        if (_fHook) {
            m_console->removeListener(_fHook);
            _fHook = nullptr;
        }
        m_console = nullptr;
    }
}

void DevConsoleView::update(const f32& dt) {
    // Blinking Logic
    m_blinkTimeRemaining -= dt;
    if (m_blinkTimeRemaining < 0) {
        m_blinkTimeRemaining = DEV_CONSOLE_MARKER_BLINK_DELAY;
        m_isViewModified = true;
    }

    if (m_currentLine != m_console->getCurrentLine()) {
        m_currentLine = m_console->getCurrentLine();
        m_isViewModified = true;
    }

    if (m_isViewModified) redrawBatch();
}

void DevConsoleView::render(const f32v2& position, const f32v2& screenSize) {
    // Check For A Batch
    if (!m_batch) return;

    redrawBatch(); // TODO(Ben): Not every frame
    m_batch->render(screenSize, &vg::SamplerState::POINT_WRAP, &vg::DepthState::NONE, &vg::RasterizerState::CULL_NONE);
}

void DevConsoleView::setBackColor(const color4& color) {
    m_backColor = color;
    m_isViewModified = true;
}

void DevConsoleView::setFontColor(const color4& color) {
    m_fontColor = color;
    m_isViewModified = true;
}

void DevConsoleView::setPosition(const f32v2& pos) {
    m_position = pos;
    m_isViewModified = true;
}

void DevConsoleView::setDimensions(const f32v2& dims) {
    m_dimensions = dims;
    m_isViewModified = true;
}

void DevConsoleView::onNewCommand(const nString& str) {
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, '\n')) {
        m_renderRing.push(item);
    }

    m_isViewModified = true;
}

void DevConsoleView::redrawBatch() {
    if (!m_batch || !m_font) return;

    m_batch->begin();

    // TODO: Draw Background
    f32 textHeight = (f32)m_font->getFontHeight();

    f32 yOffset = m_renderRing.size() * textHeight;

    // Draw Command Lines
    size_t i;
    for (i = 0; i < m_renderRing.size(); i++) {
        const cString cStr = m_renderRing.at(m_renderRing.size() - i - 1).c_str();
        if (cStr) {
            m_batch->drawString(m_font, cStr,
                m_position + f32v2(0.0f, textHeight * i + 10.0f - yOffset),
                f32v2(1),
                color4(0, 255, 0, 255),
                vg::TextAlign::TOP_LEFT,
                0.9f);
        }
    }
    // Draw current line
    if (m_currentLine.size()) {
        m_batch->drawString(m_font, m_currentLine.c_str(),
                            m_position + f32v2(0.0f, textHeight * i + 10.0f - yOffset),
                            f32v2(1),
                            color4(0, 255, 0, 255),
                            vg::TextAlign::TOP_LEFT,
                            0.9f);
    }

    // TODO: Draw Input
    m_batch->end(vg::SpriteSortMode::BACK_TO_FRONT);
}
