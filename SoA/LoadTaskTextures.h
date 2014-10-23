#pragma once
#include "LoadMonitor.h"

#include "GameManager.h"
#include "TexturePackLoader.h"

// TODO(Ben): Multiple loader threads
class LoadTaskTextures : public ILoadTask {
    virtual void load() {
        //load the texture pack
        GameManager::texturePackLoader->loadAllTextures();
    }
};