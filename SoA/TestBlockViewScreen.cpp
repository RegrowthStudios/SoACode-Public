#include "stdafx.h"
#include "TestBlockViewScreen.h"

i32 TestBlockView::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 TestBlockView::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestBlockView::build() {
    // Empty
}
void TestBlockView::destroy(const GameTime& gameTime) {
    // Empty
}

void TestBlockView::onEntry(const GameTime& gameTime) {
    // Empty
}
void TestBlockView::onExit(const GameTime& gameTime) {
    // Empty
}

void TestBlockView::onEvent(const SDL_Event& e) {
    // Empty
}
void TestBlockView::update(const GameTime& gameTime) {
    // Empty
}
void TestBlockView::draw(const GameTime& gameTime) {
    // Empty
}
