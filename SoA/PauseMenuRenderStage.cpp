#include "stdafx.h"
#include "PauseMenuRenderStage.h"

#include "PauseMenu.h"

PauseMenuRenderStage::PauseMenuRenderStage(const PauseMenu* pauseMenu) :
    _pauseMenu(pauseMenu) {
     // Empty
}

void PauseMenuRenderStage::render() {
    if (_pauseMenu->isOpen()) {
        _pauseMenu->draw();
    }
}