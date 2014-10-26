// 
//  PDA.h
//  Seed Of Andromeda
//
//  Created by Ben Arnold on 26 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides the implementation for the
//  PDA.
//

#pragma once

#ifndef PDA_H_
#define PDA_H_

#include "Computer.h"

#include "AwesomiumInterface.h"
#include "PdaAwesomiumAPI.h"

class GamePlayScreen;

#include <SDL/SDL.h>

enum class PdaState { BIOMETRICS, INVENTORY, DATA, CODEX, COMMUNICATIONS, SCANNER };

class PDA : public Computer
{
public:
    PDA();
    ~PDA();

    /// Initializes the PDA
    void init(GamePlayScreen* ownerScreen);

    /// Updates the PDA
    void update();

    /// Draws the PDA
    void draw();

    /// Handles an event
    /// @param e: The event to handle
    void onEvent(const SDL_Event& e);
private:

    vui::AwesomiumInterface<PdaAwesomiumAPI> _awesomiumInterface; ///< The user interface

    PdaAwesomiumAPI _api; ///< The callback API for the user interface

};

#endif // PDA_H_