/// 
///  SkyboxRenderer.h
///  Seed of Andromeda
///
///  Created by Ben Arnold on 28 Oct 2014
///  Copyright 2014 Regrowth Studios
///  All Rights Reserved
///  
///  This file provides a class to render a skybox
///

#pragma once

#ifndef SkyboxRenderer_h__
#define SkyboxRenderer_h__

#include <Vorb/graphics/GLProgram.h>

#include "Texture2d.h"

class SkyboxVertex {
public:
    f32v3 position;
    f32v3 texCoords;
};

class SkyboxRenderer
{
public:
    SkyboxRenderer();
    ~SkyboxRenderer();

    /// Draw the skybox
    void drawSkybox(const f32m4& VP, VGTexture textureArray);

    /// Frees the skybox mesh and shader
    void destroy();
private:

    /// Initializes the _vbo and _ibo buffers
    void initShader();
    void initBuffers();

    ui32 m_vao = 0;
    ui32 m_vbo = 0;
    ui32 m_ibo = 0;

    vg::GLProgram m_program;
};

#endif // SkyboxRenderer_h__
