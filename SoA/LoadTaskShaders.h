#pragma once
#include <SDL\SDL.h>


#include "SpriteBatch.h"
#include "Errors.h"
#include "LoadMonitor.h"
#include "GLProgramManager.h"

// TODO(Ben): Somehow make this asynchronous
class LoadTaskShaders : public ILoadTask {
    // So that these classes can call load()
    friend class LoadScreen;
    friend class GamePlayScreen;
public:
    LoadTaskShaders() {
        // Empty
    }
private:
    virtual void load() {
        vcore::GLProgramManager* glProgramManager = GameManager::glProgramManager;

        //***** Attribute Vectors ******
        // So that the same VAO can be used for multiple shaders,
        // it is advantageous to manually set the attributes so that
        // they are the same between several shaders. Otherwise they will
        // be set automatically.

        // Attributes for terrain shaders
        std::vector<nString> terrainAttribs;
        terrainAttribs.push_back("vertexPosition_modelspace");
        terrainAttribs.push_back("vertexUV");
        terrainAttribs.push_back("vertexNormal_modelspace");
        terrainAttribs.push_back("vertexColor");
        terrainAttribs.push_back("vertexSlopeColor");
        terrainAttribs.push_back("texTempRainSpec");

        // Attributes for block shaders
        std::vector<nString> blockAttribs;
        blockAttribs.push_back("position_TextureType");
        blockAttribs.push_back("uvs_animation_blendMode");
        blockAttribs.push_back("textureAtlas_textureIndex");
        blockAttribs.push_back("textureDimensions");
        blockAttribs.push_back("color_waveEffect");
        blockAttribs.push_back("overlayColor");
        blockAttribs.push_back("light_sunlight");
        blockAttribs.push_back("normal");

        /***** Texture2D *****/
        glProgramManager->addProgram("Texture2D",
                                     "Shaders/TextureShading/Texture2dShader.vert",
                                     "Shaders/TextureShading/Texture2dShader.frag");
        /***** GroundFromAtmosphere *****/
        glProgramManager->addProgram("GroundFromAtmosphere",
                                    "Shaders/TerrainShading/GroundFromAtmosphere.vert",
                                    "Shaders/TerrainShading/GroundFromAtmosphere.frag",
                                    &terrainAttribs);
        /***** SkyFromAtmosphere *****/
        glProgramManager->addProgram("SkyFromAtmosphere",
                                     "Shaders/AtmosphereShading/SkyFromAtmosphere.vert",
                                     "Shaders/AtmosphereShading/SkyFromAtmosphere.frag");
        /***** GroundFromSpace *****/
        glProgramManager->addProgram("GroundFromSpace",
                                     "Shaders/TerrainShading/GroundFromSpace.vert",
                                     "Shaders/TerrainShading/GroundFromSpace.frag",
                                     &terrainAttribs);
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
                                     "Shaders/BlockShading/standardShading.frag",
                                     &blockAttribs);
        /***** Cutout *****/
        glProgramManager->addProgram("Cutout",
                                     "Shaders/BlockShading/standardShading.vert",
                                     "Shaders/BlockShading/cutoutShading.frag",
                                     &blockAttribs);
        /***** Transparency *****/
        glProgramManager->addProgram("Transparency",
                                     "Shaders/BlockShading/standardShading.vert",
                                     "Shaders/BlockShading/transparentShading.frag",
                                     &blockAttribs);
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
                                     "Shaders/BlockShading/sonarShading.frag",
                                     &blockAttribs);
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