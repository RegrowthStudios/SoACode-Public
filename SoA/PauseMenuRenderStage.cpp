#include "stdafx.h"
#include "PauseMenuRenderStage.h"

#include "PauseMenu.h"

PauseMenuRenderStage::PauseMenuRenderStage(const PauseMenu* pauseMenu) :
    _pauseMenu(pauseMenu) {
     // Empty
}

void PauseMenuRenderStage::draw() {
    if (_pauseMenu->isOpen()) {
        _pauseMenu->draw();
    }
}