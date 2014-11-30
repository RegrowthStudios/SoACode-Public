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

    /// Opens the PDA
    void open();

    /// Closes the PDA
    void close();

    /// Updates the PDA
    void update();

    /// Draws the PDA
    void draw() const;

    /// Handles an event
    /// @param e: The event to handle
    void onEvent(const SDL_Event& e);

    /// Frees all resources
    void destroy();

    /// Returns true if it is open
    bool isOpen() const { return _isOpen; }
private:

    vui::AwesomiumInterface<PdaAwesomiumAPI> _awesomiumInterface; ///< The user interface
    
    bool _isOpen;
};

#endif // PDA_H_