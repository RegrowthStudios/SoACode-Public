#pragma once
#include "LoadMonitor.h"
#include "GameManager.h"
#include "TexturePackLoader.h"
#include "Options.h"
#include "PlanetData.h"

// TODO(Ben): Multiple loader threads
class LoadTaskTextures : public ILoadTask {
    virtual void load() {
      
        //load the texture pack
        GameManager::texturePackLoader->setColorMaps(&PlanetGenData::colorMaps);
        GameManager::texturePackLoader->loadAllTextures("Textures/TexturePacks/" + graphicsOptions.currTexturePack + "/");
    }
};