#pragma once
#include <SDL\SDL.h>

#include "Shader.h"
#include "SpriteBatch.h"
#include "Errors.h"
#include "LoadMonitor.h"
#include "GLProgramManager.h"

// TODO(Ben): Somehow make this asynhronous
class LoadTaskShaders : public ILoadTask {
    friend class LoadScreen;
public:
    LoadTaskShaders() {
        // Empty
    }
private:
    virtual void load() {
        vcore::GLProgramManager* glProgramManager = GameManager::glProgramManager;

        /***** GroundFromAtmosphere *****/
        glProgramManager->addProgram("GroundFromAtmosphere",
                                    "Shaders/TerrainShading/GroundFromAtmosphere.vert",
                                    "Shaders/TerrainShading/GroundFromAtmosphere.frag");
        /***** SkyFromAtmosphere *****/
        glProgramManager->addProgram("SkyFromAtmosphere",
                                     "Shaders/AtmosphereShading/SkyFromAtmosphere.vert",
                                     "Shaders/AtmosphereShading/SkyFromAtmosphere.frag");
        /***** GroundFromSpace *****/
        glProgramManager->addProgram("GroundFromSpace",
                                     "Shaders/TerrainShading/GroundFromSpace.vert",
                                     "Shaders/TerrainShading/GroundFromSpace.frag");
        /***** SkyFromSpace *****/
        glProgramManager->addProgram("SkyFromSpace",
                                     "Shaders/AtmosphereShading/SkyFromSpace.vert",
                                     "Shaders/AtmosphereShading/SkyFromSpace.frag");
     

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