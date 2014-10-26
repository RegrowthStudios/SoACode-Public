// 
//  GamePlayScreen.h
//  Seed Of Andromeda
//
//  Created by Ben Arnold on 17 Oct 2014
//  Copyright 2014 Regrowth Studios
//  All Rights Reserved
//  
//  This file provides the implementation for the main
//  gameplay screen. This will encompass all in game
//  behavior.
//

#pragma once

#ifndef GAMEPLAYSCREEN_H_
#define GAMEPLAYSCREEN_H_

#include "IGameScreen.h"

#include "AwesomiumInterface.h"
#include "MainMenuAPI.h"
#include "Random.h"
#include "LoadMonitor.h"
#include "PDA.h"

class App;
struct TerrainMeshMessage;

class GamePlayScreen : public IAppScreen<App>
{
    friend class PdaAwesomiumAPI;
public:
    CTOR_APP_SCREEN_DECL(GamePlayScreen, App);

    virtual i32 getNextScreen() const;
    virtual i32 getPreviousScreen() const;

    virtual void build();
    virtual void destroy(const GameTime& gameTime);

    virtual void onEntry(const GameTime& gameTime);
    virtual void onExit(const GameTime& gameTime);

    virtual void onEvent(const SDL_Event& e);
    virtual void update(const GameTime& gameTime);
    virtual void draw(const GameTime& gameTime);

private:

    /// Handles updating state based on input
    void handleInput();

    /// Handles click input for player
    void onMouseDown(const SDL_Event& e);

    /// Draws the voxel world
    void drawVoxelWorld();

    /// Updates the player
    void updatePlayer();

    /// The function that runs on the update thread. It handles
    /// loading the planet in the background.
    void updateThreadFunc();

    /// Processes messages from the update->render thread
    void processMessages();

    Player* _player; ///< The current player

    PDA _pda; ///< The PDA

    bool _inFocus; ///< true when the window is in focus

    //Camera _voxelCamera; ///< The camera for rendering the voxels
    //Camera _planetCamera; ///< The camera for rendering the planet

    std::thread* _updateThread; ///< The thread that updates the planet. Runs updateThreadFunc()
    volatile bool _threadRunning; ///< True when the thread should be running
};

#endif // GAMEPLAYSCREEN_H_