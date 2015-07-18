///
/// ChunkGridRenderStage.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 14 Nov 2014
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// This file provides the chunk grid render stage,
/// which is a debug rendering stage for chunk state
///

#pragma once

#ifndef ChunkGridRenderStage_h__
#define ChunkGridRenderStage_h__

#include "IRenderStage.h"
#include "MTRenderState.h"

#include <Vorb/graphics/GLProgram.h>

class ChunkGrid;
class GameRenderParams;
class ChunkMemoryManager;

struct ChunkGridVertex {
public:
    f32v3 position;
    f32v2 uv;
    color4 color;
};

class ChunkGridRenderStage : public IRenderStage {
public:
    void hook(const GameRenderParams* gameRenderParams);

    // Draws the render stage
    void setState(const MTRenderState* state) { m_state = state; }

    virtual void init(vui::GameWindow* window, StaticLoadContext& context) override;
    virtual void render(const Camera* camera) override;
    virtual void dispose(StaticLoadContext& context) override;
private:
    void drawGrid(std::vector<ChunkGridVertex> vertices, std::vector<ui32> indices);

    vg::GLProgram m_program;
    const GameRenderParams* m_gameRenderParams = nullptr; ///< Handle to some shared parameters
    const MTRenderState* m_state = nullptr;

    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_vao;
};

#endif // ChunkGridRenderStage_h__
