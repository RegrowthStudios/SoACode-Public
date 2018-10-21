#include "stdafx.h"
#include "PauseMenuRenderStage.h"

#include "PauseMenu.h"

void PauseMenuRenderStage::hook(const PauseMenu* pauseMenu) {
    _pauseMenu = pauseMenu;
}

void PauseMenuRenderStage::render(const Camera* camera VORB_MAYBE_UNUSED /*= nullptr*/) {
    if (_pauseMenu->isOpen()) {
        _pauseMenu->draw();
    }
}