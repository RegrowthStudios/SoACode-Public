/// 
///  SonarRenderStage.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file implements the sonar render stage which does a cool
///  sonar rendering effect.
///

#pragma once

#ifndef SonarRenderStage_h__
#define SonarRenderStage_h__

#include <Vorb/IRenderStage.h>

class Camera;
class GameRenderParams;
class MeshManager;

class SonarRenderStage : public vg::IRenderStage
{
public:
    /// Constructor which injects dependencies
    /// @param camera: The camera handle
    /// @param meshManager: Handle to the class that holds meshes
    SonarRenderStage(const GameRenderParams* gameRenderParams);

    // Draws the render stage
    virtual void draw() override;
private:
    const GameRenderParams* _gameRenderParams; ///< Handle to shared parameters
};

#endif // SonarRenderStage_h__
