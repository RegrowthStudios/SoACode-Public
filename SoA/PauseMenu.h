///
/// PauseMenu.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 30 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Pause menu implementation for use in GamePlayScreen
///

#pragma once

#ifndef PauseMenu_h__
#define PauseMenu_h__

#include "PauseMenuAwesomiumAPI.h"
#include "AwesomiumInterface.h"

class PauseMenu {
public:
    PauseMenu();
    ~PauseMenu();

    void open();
    void close();

    const bool& isOpen() const { return _isOpen; }
private:
    vui::AwesomiumInterface<PauseMenuAwesomiumAPI> _awesomiumInterface; ///< The user interface

    PauseMenuAwesomiumAPI _api; ///< The callback API for the user interface

    bool _isOpen;
};

#endif // PauseMenu_h__