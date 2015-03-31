#include "stdafx.h"
#include "PauseMenu.h"

#include "GameplayScreen.h"

#include <Vorb/graphics/ShaderManager.h>

PauseMenu::PauseMenu() {
    // Empty
}

PauseMenu::~PauseMenu() {
    // Empty
}

void PauseMenu::init(GameplayScreen* ownerScreen) {
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
    if (_isOpen) {
        _awesomiumInterface.invokeFunction("close");
        _isOpen = false;
    }
}

void PauseMenu::update() {
    _awesomiumInterface.update();
}

void PauseMenu::draw() const {
    if (!m_program) vg::ShaderManager::createProgramFromFile("Shaders/TextureShading/Texture2dShader.vert",
                                                             "Shaders/TextureShading/Texture2dShader.frag");
    _awesomiumInterface.draw(m_program);
}

void PauseMenu::destroy() {
    _awesomiumInterface.destroy();
}