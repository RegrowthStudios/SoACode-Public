#include "stdafx.h"
#include "SoaController.h"

#include "SoaState.h"
#include "GameSystemFactories.h"

void SoaController::startGame(OUT SoaState* state) {
    if (state->isNewGame) {
        // Create the player entity
        state->playerEntity = GameSystemFactories::createPlayer(&state->gameSystem, f64v3(0.0),
                                          f64q(), 73.0f, f64v3(0.0));
    } else {

    }
}
