///
/// SoaController.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 11 Jan 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Main game controller for SoA
///

#pragma once

#ifndef SoaController_h__
#define SoaController_h__

class SoaState;

class SoaController {
public:
    ~SoaController();
    void startGame(OUT SoaState* state);
};

#endif // SoaController_h__
