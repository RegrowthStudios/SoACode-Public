#include "stdafx.h"
#include "PauseMenu.h"

#include "GamePlayScreen.h"

PauseMenu::PauseMenu() {
    // Empty
}

PauseMenu::~PauseMenu() {
    // Empty
}

void PauseMenu::init(GamePlayScreen* ownerScreen) {
    // Initialize the user interface
    _awesomiumInterface.init("UI/PauseMenu/",
                             "PAUSE_UI",
                             "pause.html",
                             ownerScreen->getWindowWidth(),
                             ownerScreen->getWindowHeight(),
                             ownerScreen);
}

void PauseMenu::open() {
    _awesomiumInterface.invokeFunction("openInventory");
    _isOpen = true;
}

void PauseMenu::close() {
    _awesomiumInterface.invokeFunction("close");
    _isOpen = false;
}

void PauseMenu::update() {
    _awesomiumInterface.update();
}

void PauseMenu::draw() const {
    _awesomiumInterface.draw(GameManager::glProgramManager->getProgram("Texture2D"));
}

void PauseMenu::onEvent(const SDL_Event& e) {
    _awesomiumInterface.handleEvent(e);
}

void PauseMenu::destroy() {
    _awesomiumInterface.destroy();
}