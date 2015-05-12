#include "stdafx.h"
#include "MainMenuForm.h"
#include "MainMenuScreen.h"


MainMenuForm::MainMenuForm() {

}

MainMenuForm::~MainMenuForm() {

}

void MainMenuForm::init(MainMenuScreen* ownerScreen, ui32v4 destRect, vg::SpriteFont* defaultFont /*= nullptr*/, vg::SpriteBatch* spriteBatch /*= nullptr*/) {
    Form::init(ownerScreen, destRect, defaultFont, spriteBatch);
    m_ownerScreen = ownerScreen;
}

bool MainMenuForm::registerCallback(vui::Widget* w, nString callback) {
    
    return false;
}
