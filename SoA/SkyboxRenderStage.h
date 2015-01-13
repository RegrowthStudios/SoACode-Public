/// 
///  SkyboxRenderStage.h
///  Seed of Andromeda
///
///  Created by Benjamin Arnold on 1 Nov 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file provides the implementation for the skybox render stage
///

#pragma once

#ifndef SkyboxRenderStage_h__
#define SkyboxRenderStage_h__

#include <Vorb/graphics/GLProgram.h>
#include <Vorb/graphics/IRenderStage.h>

class SkyboxRenderer;
class Camera;

class SkyboxRenderStage : public vg::IRenderStage
{
public:
    /// Constructor which injects dependencies
    /// @param program: The opengl program for rendering
    /// @param camera: Handle to the camera used to render the stage
    SkyboxRenderStage(vg::GLProgram* program,
                     const Camera* camera);
    ~SkyboxRenderStage();

    // Draws the render stage
    virtual void draw() override;
private:
    void drawSpace(glm::mat4 &VP);
    // Temporary until we have an actual sun
    void drawSun(float theta, glm::mat4 &MVP);
    // Update projection matrix
    void updateProjectionMatrix();

    SkyboxRenderer* _skyboxRenderer; ///< Renders the skybox
    vg::GLProgram* _glProgram; ///< Program used for rendering

    f32m4 _projectionMatrix; ///< Projection matrix for the skybox
    float _fieldOfView; ///< Current field of view for the camera
    float _aspectRatio; ///< Current aspect ratio for the camera
};

#endif // SkyboxRenderStage_h__
