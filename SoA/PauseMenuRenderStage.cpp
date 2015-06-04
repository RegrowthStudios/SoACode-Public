#include "stdafx.h"
#include "PauseMenuRenderStage.h"

#include "PauseMenu.h"

PauseMenuRenderStage::PauseMenuRenderStage() {
     // Empty
}

void PauseMenuRenderStage::init(const PauseMenu* pauseMenu) {
    _pauseMenu = pauseMenu;
}

void PauseMenuRenderStage::render() {
    if (_pauseMenu->isOpen()) {
        _pauseMenu->draw();
    }
}