/// 
///  PlanetRenderStage.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  MIT License
///  
///  This file implements the planet rendering stage, which renders
///  planets and their atmospheres.
///

#pragma once

#ifndef PlanetRenderStage_h__
#define PlanetRenderStage_h__

#include <Vorb/graphics/IRenderStage.h>

class Camera;

class PlanetRenderStage : public vg::IRenderStage
{
public:
    /// Constructor which injects dependencies
    /// @param camera: The camera handle
    PlanetRenderStage(const Camera* camera);

    /// Draws the render stage
    virtual void draw() override;
};
#endif // PlanetRenderStage_h__

