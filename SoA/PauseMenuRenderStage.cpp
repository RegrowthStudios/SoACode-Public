#include "stdafx.h"
#include "PauseMenuRenderStage.h"

#include "PauseMenu.h"

void PauseMenuRenderStage::init(const PauseMenu* pauseMenu) {
    _pauseMenu = pauseMenu;
}

void PauseMenuRenderStage::render(const Camera* camera) {
    if (_pauseMenu->isOpen()) {
        _pauseMenu->draw();
    }
}