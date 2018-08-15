#include "stdafx.h"
#include "TestNewBlockAPIScreen.h"

#include "Positional.h"

i32 TestNewBlockAPIScreen::getNextScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}
i32 TestNewBlockAPIScreen::getPreviousScreen() const {
    return SCREEN_INDEX_NO_SCREEN;
}

void TestNewBlockAPIScreen::build() {
    // Empty
}
void TestNewBlockAPIScreen::destroy(const vui::GameTime& gameTime VORB_UNUSED) {
    // Empty
}

void TestNewBlockAPIScreen::onEntry(const vui::GameTime& gameTime VORB_UNUSED) {
    VoxelIterablePosition pos { 0, 1, 2 };
    printf("Pos: %d,%d,%d\n", pos.x, pos.y, pos.z);
    pos.x += 5;
    printf("Pos: %d,%d,%d\n", pos.x, pos.y, pos.z);
    pos.x += 35;
    printf("Pos: %d,%d,%d\n", pos.x, pos.y, pos.z);
    pos.wx() += 0;
    printf("Pos: %d,%d,%d\n", pos.x, pos.y, pos.z);
    pos.cx() += 200;
    printf("Pos: %d,%d,%d\n", pos.x, pos.y, pos.z);
    (pos.wx() += 200).rz() += 200;
    printf("Pos: %d,%d,%d\n", pos.x, pos.y, pos.z);
}
void TestNewBlockAPIScreen::onExit(const vui::GameTime& gameTime VORB_UNUSED) {
    // Empty
}

void TestNewBlockAPIScreen::update(const vui::GameTime& gameTime VORB_UNUSED) {
    // Empty
}
void TestNewBlockAPIScreen::draw(const vui::GameTime& gameTime VORB_UNUSED) {
    // Empty
}
