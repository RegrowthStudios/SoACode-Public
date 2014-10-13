#include "stdafx.h"
#include "LoadTaskInput.h"

#include "GameManager.h"
#include "InputManager.h"
#include "Inputs.h"

void LoadTaskInput::load() {
    Sleep(2000);
    initInputs();
    GameManager::inputManager->loadAxes();
}
