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

class App;
struct SoaState;

class SoaController {
public:
    SoaController(const App* app);
    ~SoaController();
    void startGame(OUT SoaState* state);
private:
    const App* m_app = nullptr; ///< App for querying things like aspect ratio
};

#endif // SoaController_h__
