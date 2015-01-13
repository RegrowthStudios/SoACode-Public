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
    f32v2 texCoords;
};

class SkyboxRenderer
{
public:
    SkyboxRenderer();
    ~SkyboxRenderer();

    /// Draw the skybox
    void drawSkybox(vg::GLProgram* program, const f32m4& VP, vg::Texture textures[]);

    /// Frees the skybox mesh
    void destroy();

private:

    /// Initializes the _vbo and _ibo buffers
    void initBuffers(vg::GLProgram* program);

    ui32 _vao;
    ui32 _vbo;
    ui32 _ibo;

};

#endif // SkyboxRenderer_h__
