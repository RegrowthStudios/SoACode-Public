#pragma once
#include <SDL\SDL.h>


#include "SpriteBatch.h"
#include "Errors.h"
#include "LoadMonitor.h"
#include "GLProgramManager.h"

// TODO(Ben): Somehow make this asynchronous
class LoadTaskShaders : public ILoadTask {
    friend class LoadScreen;
public:
    LoadTaskShaders() {
        // Empty
    }
private:
    virtual void load() {
        vcore::GLProgramManager* glProgramManager = GameManager::glProgramManager;

        /***** Texture2D *****/
        glProgramManager->addProgram("Texture2D",
                                     "Shaders/TextureShading/Texture2dShader.vert",
                                     "Shaders/TextureShading/Texture2dShader.frag");
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
        /***** Texture *****/
        glProgramManager->addProgram("Texture",
                                     "Shaders/TextureShading/TextureShading.vert",
                                     "Shaders/TextureShading/TextureShading.frag");
        /***** BasicColor *****/
        glProgramManager->addProgram("BasicColor",
                                     "Shaders/BasicShading/BasicColorShading.vert",
                                     "Shaders/BasicShading/BasicColorShading.frag");
        /***** HDR *****/
        glProgramManager->addProgram("HDR",
                                     "Shaders/PostProcessing/PassThrough.vert",
                                     "Shaders/PostProcessing/HDRShader.frag");
        /***** MotionBlur *****/
        glProgramManager->addProgram("MotionBlur",
                                     "Shaders/PostProcessing/PassThrough.vert",
                                     "Shaders/PostProcessing/MotionBlur.frag");
        /***** Block *****/
        glProgramManager->addProgram("Block",
                                     "Shaders/BlockShading/standardShading.vert",
                                     "Shaders/BlockShading/standardShading.frag");
        /***** Cutout *****/
        glProgramManager->addProgram("Cutout",
                                     "Shaders/BlockShading/standardShading.vert",
                                     "Shaders/BlockShading/cutoutShading.frag");
        /***** Transparency *****/
        glProgramManager->addProgram("Transparency",
                                     "Shaders/BlockShading/standardShading.vert",
                                     "Shaders/BlockShading/transparentShading.frag");
        /***** Water *****/
        glProgramManager->addProgram("Water",
                                     "Shaders/WaterShading/WaterShading.vert",
                                     "Shaders/WaterShading/WaterShading.frag");
        /***** Billboard *****/
        glProgramManager->addProgram("Billboard",
                                     "Shaders/BillboardShading/BillboardShading.vert",
                                     "Shaders/BillboardShading/BillboardShading.frag");
        /***** FixedSizeBillboard *****/
        glProgramManager->addProgram("FixedSizeBillboard",
                                     "Shaders/BillboardShading/FixedSizeBillboardShading.vert",
                                     "Shaders/BillboardShading/FixedSizeBillboardShading.frag");
        /***** Sonar *****/
        glProgramManager->addProgram("Sonar",
                                     "Shaders/BlockShading/standardShading.vert",
                                     "Shaders/BlockShading/sonarShading.frag");
        /***** PhysicsBlock *****/
        glProgramManager->addProgram("PhysicsBlock",
                                     "Shaders/PhysicsBlockShading/PhysicsBlockShading.vert",
                                     "Shaders/BlockShading/standardShading.frag");
        /***** TreeBillboard *****/
        glProgramManager->addProgram("TreeBillboard",
                                     "Shaders/TreeBillboardShading/TreeBillboardShading.vert",
                                     "Shaders/TreeBillboardShading/TreeBillboardShading.frag");

        checkGlError("InitializeShaders()");

    }
};