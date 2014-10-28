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

class GameRenderStage : public vg::IRenderStage
{
public:
    GameRenderStage();
    ~GameRenderStage();

    void render() override;
private:
    
    void drawSpace(glm::mat4 &VP, bool connectedToPlanet);
    void drawPlanets();
    void drawVoxels();

};

#endif // GameRenderStage_h__