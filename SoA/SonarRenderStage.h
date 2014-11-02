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

#include "IRenderStage.h"

class Camera;
class MeshManager;

class SonarRenderStage : public vg::IRenderStage
{
public:
    /// Constructor which injects dependencies
    /// @param camera: The camera handle
    /// @param meshManager: Handle to the class that holds meshes
    SonarRenderStage(const Camera* camera, 
                     const MeshManager* meshManager);

    // Draws the render stage
    virtual void draw() override;
private:
    const MeshManager* _meshManager; ///< Stores the meshes we need to render
};

#endif // SonarRenderStage_h__
