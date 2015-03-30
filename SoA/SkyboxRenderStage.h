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
    /// @param camera: Handle to the camera used to render the stage
    SkyboxRenderStage(const Camera* camera);
    ~SkyboxRenderStage();

    // Draws the render stage
    virtual void render() override;
private:
    void buildShaders();
    void drawSpace(glm::mat4 &VP);
    // Temporary until we have an actual sun
    void drawSun(float theta, glm::mat4 &MVP);
    // Update projection matrix
    void updateProjectionMatrix();

    SkyboxRenderer* m_skyboxRenderer; ///< Renders the skybox
    vg::GLProgram* m_program = nullptr; ///< Program used for rendering

    f32m4 m_projectionMatrix; ///< Projection matrix for the skybox
    float m_fieldOfView; ///< Current field of view for the camera
    float m_aspectRatio; ///< Current aspect ratio for the camera
};

#endif // SkyboxRenderStage_h__
