#include "stdafx.h"
#include "PDA.h"

#include <Vorb/graphics/GLProgram.h>

#include "GameplayScreen.h"
#include "ShaderLoader.h"

PDA::PDA() {
    // Empty
}
PDA::~PDA() {
    // Empty
}

void PDA::init(GameplayScreen* ownerScreen) {
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
    if (!m_program) ShaderLoader::createProgramFromFile("Shaders/TextureShading/Texture2dShader.vert",
                                                             "Shaders/TextureShading/Texture2dShader.frag");
    _awesomiumInterface.draw(m_program);
}

void PDA::destroy() {
    _awesomiumInterface.destroy();
}