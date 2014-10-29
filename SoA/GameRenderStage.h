/// 
///  GameRenderStage.h
///  Vorb Engine
///
///  Created by Ben Arnold on 28 Oct 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file provides an interface for a render pipeline
///  stage.
///

#pragma once

#ifndef GameRenderStage_h__
#define GameRenderStage_h__

#include "IRenderStage.h"

class Camera;

class GameRenderStage : public vg::IRenderStage
{
public:
    GameRenderStage(vg::GLProgramManager* glProgramManager,
                    vg::TextureCache* textureCache,
                    Camera* chunkCamera,
                    Camera* worldCamera);
    ~GameRenderStage();

    void render() override;
private:
    
    void drawSpace(glm::mat4 &VP, bool connectedToPlanet);
    void drawPlanets();
    void drawVoxels();

    Camera* _chunkCamera; ///< Camera that renders the chunks
    Camera* _worldCamera; ///< Camera that renders the worlds

    vg::GLProgramManager* _glProgramManager; ///< Holds the needed shaders
    vg::TextureCache* _textureCache; ///< Holds the needed textures

    vg::Texture* _skyboxTetxures[6]; ///< The skybox textures duh
};

#endif // GameRenderStage_h__