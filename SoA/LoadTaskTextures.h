#pragma once
#include "LoadMonitor.h"
#include "GameManager.h"
#include "SoaOptions.h"
#include "PlanetData.h"

// TODO(Ben): Multiple loader threads
class LoadTaskTextures : public ILoadTask {
    virtual void load() {
      
        //load the texture pack
        //GameManager::texturePackLoader->setColorMaps(&PlanetGenData::colorMaps);
        //GameManager::texturePackLoader->loadAllTextures("Textures/TexturePacks/" + soaOptions.getStringOption("Texture Pack").value + "/");
    }
};