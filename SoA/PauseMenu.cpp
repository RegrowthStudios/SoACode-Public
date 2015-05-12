#include "stdafx.h"
#include "PauseMenu.h"

#include "GameplayScreen.h"
#include "ShaderLoader.h"

PauseMenu::PauseMenu() {
    // Empty
}

PauseMenu::~PauseMenu() {
    // Empty
}

void PauseMenu::init(GameplayScreen* ownerScreen) {

}

void PauseMenu::open() {
    _isOpen = true;
}

void PauseMenu::close() {
    if (_isOpen) {
        _isOpen = false;
    }
}

void PauseMenu::update() {

}

void PauseMenu::draw() const {
    if (!m_program) ShaderLoader::createProgramFromFile("Shaders/TextureShading/Texture2dShader.vert",
                                                             "Shaders/TextureShading/Texture2dShader.frag");

}

void PauseMenu::destroy() {

}