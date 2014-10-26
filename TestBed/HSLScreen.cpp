#include "stdafx.h"
#include "HSLScreen.h"

#include "SpriteBatch.h"

i32 HSLScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

i32 HSLScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void HSLScreen::build() {
}
void HSLScreen::destroy(const GameTime& gameTime) {
}

void HSLScreen::onEntry(const GameTime& gameTime) {
    _sb = new SpriteBatch();
}
void HSLScreen::onExit(const GameTime& gameTime) {
}

void HSLScreen::onEvent(const SDL_Event& e) {
}
void HSLScreen::update(const GameTime& gameTime) {
}
void HSLScreen::draw(const GameTime& gameTime) {
}
