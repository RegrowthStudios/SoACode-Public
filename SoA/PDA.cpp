#include "stdafx.h"
#include "PDA.h"

#include <Vorb/graphics/GLProgram.h>

#include "GameplayScreen.h"

PDA::PDA() {
    // Empty
}
PDA::~PDA() {
    // Empty
}

void PDA::init(GameplayScreen* ownerScreen, const vg::GLProgramManager* glProgramManager) {
    // Initialize the user interface
    _awesomiumInterface.init("UI/PDA/", 
                             "PDA_UI",
                             "index.html", 
                             ownerScreen->getWindowWidth(),
                             ownerScreen->getWindowHeight(),
                             ownerScreen);
    m_glProgramManager = glProgramManager;
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
    _awesomiumInterface.draw(m_glProgramManager->getProgram("Texture2D"));
}

void PDA::destroy() {
    _awesomiumInterface.destroy();
}