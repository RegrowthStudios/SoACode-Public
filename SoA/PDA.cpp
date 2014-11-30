#include "stdafx.h"
#include "PDA.h"

#include "GamePlayScreen.h"
#include "GLProgram.h"


PDA::PDA() : 
    _isOpen(false) {
    // Empty
}


PDA::~PDA() {
    // Empty
}

void PDA::init(GamePlayScreen* ownerScreen) {
    // Initialize the user interface
    _awesomiumInterface.init("UI/PDA/", 
                             "PDA_UI",
                             "index.html", 
                             ownerScreen->getWindowWidth(),
                             ownerScreen->getWindowHeight(),
                             ownerScreen);
}

void PDA::open() {
    _awesomiumInterface.invokeFunction("openInventory");
    _isOpen = true;
}

void PDA::close() {
    _awesomiumInterface.invokeFunction("close");
    _isOpen = false;
}

void PDA::update() {
    _awesomiumInterface.update();
}

void PDA::draw() const {
    _awesomiumInterface.draw(GameManager::glProgramManager->getProgram("Texture2D"));
}

void PDA::onEvent(const SDL_Event& e) {
    _awesomiumInterface.handleEvent(e);
}

void PDA::destroy() {
    _awesomiumInterface.destroy();
}