///
/// ColoredFullQuadRenderer.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 22 Mar 2015
/// Copyright 2014 Regrowth Studios
/// MIT License
///
/// Summary:
/// Renders a FullQuadVBO with color
///

#pragma once

#ifndef ColoredFullQuadRenderer_h__
#define ColoredFullQuadRenderer_h__

#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/VorbPreDecl.inl>

DECL_VG(class GLProgram);

class ColoredFullQuadRenderer {
public:
    ~ColoredFullQuadRenderer();
    void draw(vg::FullQuadVBO& quad, const f32v4& color);
private:
    vg::GLProgram* m_program = nullptr;
};

#endif // ColoredFullQuadRenderer_h__
