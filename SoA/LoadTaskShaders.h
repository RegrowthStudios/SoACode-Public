#pragma once
#include <SDL\SDL.h>

#include "Shader.h"
#include "SpriteBatch.h"
#include "Errors.h"
#include "LoadMonitor.h"

// TODO(Ben): Somehow make this asynhronous
class LoadTaskShaders : public ILoadTask {
    friend class LoadScreen;
public:
    LoadTaskShaders() {
        // Empty
    }
private:
    virtual void load() {

        textureShader.Initialize();
        basicColorShader.DeleteShader();
        basicColorShader.Initialize();
        simplexNoiseShader.DeleteShader();
        //simplexNoiseShader.Initialize();
        hdrShader.DeleteShader();
        motionBlurShader.DeleteShader();
        motionBlurShader.Initialize();
        hdrShader.Initialize();
        blockShader.DeleteShader();
        blockShader.Initialize();
        cutoutShader.DeleteShader();
        cutoutShader.Initialize();
        transparencyShader.DeleteShader();
        transparencyShader.Initialize();
        atmosphereToSkyShader.DeleteShader();
        atmosphereToSkyShader.Initialize();
        atmosphereToGroundShader.DeleteShader();
        atmosphereToGroundShader.Initialize();
        spaceToSkyShader.DeleteShader();
        spaceToSkyShader.Initialize();
        spaceToGroundShader.DeleteShader();
        spaceToGroundShader.Initialize();
        waterShader.DeleteShader();
        waterShader.Initialize();
        billboardShader.DeleteShader();
        billboardShader.Initialize();
        fixedSizeBillboardShader.DeleteShader();
        fixedSizeBillboardShader.Initialize();
        sonarShader.DeleteShader();
        sonarShader.Initialize();
        physicsBlockShader.DeleteShader();
        physicsBlockShader.Initialize();
        treeShader.DeleteShader();
        treeShader.Initialize();

        checkGlError("InitializeShaders()");

    }
};