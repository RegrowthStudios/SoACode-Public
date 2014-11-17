/// 
///  GamePlayScreenEvents.hpp
///  Seed of Andromeda
///
///  Created by Frank McCoy on 9 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file provides the event delegates for the GamePlayScreen
///


#pragma once

#ifndef GAME_PLAY_SCREEN_EVENTS_HPP
#define GAME_PLAY_SCREEN_EVENTS_HPP

#include "Event.hpp"
#include "GamePlayScreen.h"
#include "global.h"
#include "GameManager.h"
#include "TexturePackLoader.h"
#include "Player.h"
#include "GpuMemory.h"
#include "GLProgramManager.h"
#include "LoadTaskShaders.h"
#include "Options.h"
#include "PDA.h"
#include "types.h"
#include "GamePlayRenderPipeline.h"

/// Generic delegate for all delegates that require access tot he GamePlayScreen

// @author Frank McCoy
class GamePlayScreenDelegate: IDelegate<ui32> {
public:
    GamePlayScreenDelegate() {}
    GamePlayScreenDelegate(GamePlayScreen* screen): _screen(screen) {}

    virtual void invoke(void* sender, ui32 key) = 0;
protected:
    GamePlayScreen* _screen;
};

/// Delegate to handle when the Pause key down event
class OnPauseKeyDown: GamePlayScreenDelegate {
public:
    OnPauseKeyDown() {}
    OnPauseKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        _screen->_inFocus = false;
    }
};


/// Delegate to handle when the Fly key down event
class OnFlyKeyDown: GamePlayScreenDelegate {
public:
    OnFlyKeyDown() {}
    OnFlyKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        _screen->_player->flyToggle();
    }
};

/// Delegate to handle when the Grid key down event
class OnGridKeyDown: GamePlayScreenDelegate {
public:
    OnGridKeyDown() {}
    OnGridKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        _screen->_renderPipeline.toggleChunkGrid();
    }
};

/// Delegate to handle when the Reload Textures key down event
class OnReloadTexturesKeyDown: GamePlayScreenDelegate {
public:
    OnReloadTexturesKeyDown() {}
    OnReloadTexturesKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        // Free atlas
        vg::GpuMemory::freeTexture(blockPack.textureInfo.ID);
        // Free all textures
        GameManager::textureCache->destroy();
        // Reload textures
        GameManager::texturePackLoader->loadAllTextures("Textures/TexturePacks/" + graphicsOptions.texturePackString + "/");
        GameManager::texturePackLoader->uploadTextures();
        GameManager::texturePackLoader->writeDebugAtlases();
        GameManager::texturePackLoader->setBlockTextures(Blocks);

        GameManager::getTextureHandles();

        // Initialize all the textures for blocks.
        for(size_t i = 0; i < Blocks.size(); i++) {
            Blocks[i].InitializeTexture();
        }

        GameManager::texturePackLoader->destroy();
    }
};

/// Delegate to handle when the Reload Shdaers key down event
class OnReloadShadersKeyDown: GamePlayScreenDelegate {
public:
    OnReloadShadersKeyDown() {}
    OnReloadShadersKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        GameManager::glProgramManager->destroy();
        LoadTaskShaders shaderTask;
        shaderTask.load();
        _screen->_renderPipeline.destroy();
        _screen->initRenderPipeline();
    }
};

/// Delegate to handle when the Inventory key down event
class OnInventoryKeyDown: GamePlayScreenDelegate {
public:
    OnInventoryKeyDown() {}
    OnInventoryKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        if(_screen->_pda.isOpen()) {
            _screen->_pda.close();
            SDL_SetRelativeMouseMode(SDL_TRUE);
            _screen->_inFocus = true;
            SDL_StartTextInput();
        } else {
            _screen->_pda.open();
            SDL_SetRelativeMouseMode(SDL_FALSE);
            _screen->_inFocus = false;
            SDL_StopTextInput();
        }
    }
};

/// Delegate to handle when the Reload UI key down event
class OnReloadUIKeyDown: GamePlayScreenDelegate {
public:
    OnReloadUIKeyDown() {}
    OnReloadUIKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        if(_screen->_pda.isOpen()) {
            _screen->_pda.close();
        }
        _screen->_pda.destroy();
        _screen->_pda.init(_screen);
    }
};

/// Delegate to handle when the HUD key down event
class OnHUDKeyDown: GamePlayScreenDelegate {
public:
    OnHUDKeyDown() {}
    OnHUDKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        _screen->_renderPipeline.cycleDevHud();
    }
};

#endif //GAME_PLAY_SCREEN_EVENTS_HPP