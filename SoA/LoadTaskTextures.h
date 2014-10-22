#pragma once
#include "LoadMonitor.h"

#include "GameManager.h"
#include "TexturePackLoader.h"

// This is hacky and temporary, it does way to much
class LoadTaskTextures : public ILoadTask {
    virtual void load() {
        //load the texture pack
        GameManager::texturePackLoader->loadAllTextures();
    }
};