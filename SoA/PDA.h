// 
//  PDA.h
//  Seed Of Andromeda
//
//  Created by Ben Arnold on 26 Oct 2014
//  Copyright 2014 Regrowth Studios
//  MIT License
//  
//  This file provides the implementation for the
//  PDA.
//

#pragma once

#ifndef PDA_H_
#define PDA_H_

#include <SDL2/SDL.h>
#include <Vorb/VorbPreDecl.inl>

#include "Computer.h"

class GameplayScreen;

enum class PdaState { BIOMETRICS, INVENTORY, DATA, CODEX, COMMUNICATIONS, SCANNER };

DECL_VG(class GLProgram)

class PDA : public Computer
{
public:
    PDA();
    ~PDA();

    /// Initializes the PDA
    /// @param ownerScreen: The screen that owns this PDA
    void init(GameplayScreen* ownerScreen);

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
    vg::GLProgram* m_program = nullptr;
    bool _isOpen = false;
};

#endif // PDA_H_