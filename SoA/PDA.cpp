#include "stdafx.h"
#include "PDA.h"

#include <Vorb/graphics/GLProgram.h>

#include "GamePlayScreen.h"
#include "ShaderLoader.h"

PDA::PDA() {
    // Empty
}
PDA::~PDA() {
    // Empty
}

void PDA::init(GameplayScreen* ownerScreen) {
    // Initialize the user interface
}

void PDA::open() {

    _isOpen = true;
}

void PDA::close() {

    _isOpen = false;
}

void PDA::update() {

}

void PDA::draw() const {
    if (!m_program) ShaderLoader::createProgramFromFile("Shaders/TextureShading/Texture2dShader.vert",
                                                             "Shaders/TextureShading/Texture2dShader.frag");

}

void PDA::destroy() {

}