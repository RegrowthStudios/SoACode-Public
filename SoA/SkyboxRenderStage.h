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

#include "SkyboxRenderer.h"
#include "IRenderStage.h"
#include <Vorb/graphics/GLProgram.h>
#include <Vorb/AssetLoader.h>

class Camera;
class ModPathResolver;

class SkyboxRenderStage : public IRenderStage
{
public:
    void hook(SoaState* state);

    void load(LoadContext& context, vcore::RPCManager& glRPCM) override;

    // Draws the render stage
    virtual void render(const Camera* camera) override;
private:
    void loadSkyboxTexture();
    void drawSpace(glm::mat4 &VP);
    // Update projection matrix
    void updateProjectionMatrix(const Camera* camera);

    SkyboxRenderer m_skyboxRenderer; ///< Renders the skybox
    vg::GLProgram* m_program = nullptr; ///< Program used for rendering

    f32m4 m_projectionMatrix; ///< Projection matrix for the skybox
    float m_fieldOfView; ///< Current field of view for the camera
    float m_aspectRatio; ///< Current aspect ratio for the camera

    VGTexture m_skyboxTextureArray = 0; ///< Texture array for skybox
    const ModPathResolver* m_textureResolver = nullptr;
    vcore::GLRPC m_rpc;

};

#endif // SkyboxRenderStage_h__
