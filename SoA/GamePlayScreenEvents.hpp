#pragma once

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

class GamePlayScreenDelegate: IDelegate<ui32> {
public:
    GamePlayScreenDelegate() {}
    GamePlayScreenDelegate(GamePlayScreen* screen): _screen(screen) {}

    virtual void invoke(void* sender, ui32 key) = 0;
protected:
    GamePlayScreen* _screen;
};

class OnPauseKeyDown: GamePlayScreenDelegate {
public:
    OnPauseKeyDown() {}
    OnPauseKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        _screen->_inFocus = false;
    }
};

class OnFlyKeyDown: GamePlayScreenDelegate {
public:
    OnFlyKeyDown() {}
    OnFlyKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        _screen->_player->flyToggle();
    }
};

class OnGridKeyDown: GamePlayScreenDelegate {
public:
    OnGridKeyDown() {}
    OnGridKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        gridState = !gridState;
    }
};

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

class OnReloadShadersKeyDown: GamePlayScreenDelegate {
public:
    OnReloadShadersKeyDown() {}
    OnReloadShadersKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        GameManager::glProgramManager->destroy();
        LoadTaskShaders shaderTask;
        shaderTask.load();
    }
};

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

class OnHUDKeyDown: GamePlayScreenDelegate {
public:
    OnHUDKeyDown() {}
    OnHUDKeyDown(GamePlayScreen* screen): GamePlayScreenDelegate(screen) {}

    virtual void invoke(void* sender, ui32 key) {
        _renderPipeline.cycleDevHud();
    }
};