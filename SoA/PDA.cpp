#include "stdafx.h"
#include "PDA.h"

#include "GamePlayScreen.h"


PDA::PDA() {
    // Empty
}


PDA::~PDA() {
    // Empty
}

void PDA::init(GamePlayScreen* ownerScreen) {
    // Initialize the user interface
    _awesomiumInterface.init("UI/PDA/", "PDA_UI", "index.html", graphicsOptions.screenWidth, graphicsOptions.screenHeight, &_api, ownerScreen);
}

void PDA::update() {

}

void PDA::onEvent(const SDL_Event& e) {

}

void PDA::draw() {

}