#pragma once
#include "LoadMonitor.h"
#include "BlockData.h"
#include "FileSystem.h"

// Sample Dependency Task
class LoadBlockDataTask : public ILoadTask {
    virtual void load() {
        initConnectedTextures();

        if (!(fileManager.loadBlocks("Data/BlockData.ini"))) exit(123432);
        //    cout << SDL_GetTicks() - stt << endl;
        fileManager.saveBlocks("Data/test.ini");

        LoadTextures();

        SetBlockAvgTexColors();
    }
};