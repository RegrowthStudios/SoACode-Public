///
/// ColorFilterRenderStage.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 25 Apr 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Renders a full screen color filter
///

#pragma once

#ifndef ColorFilterRenderStage_h__
#define ColorFilterRenderStage_h__

#include <Vorb/graphics/FullQuadVBO.h>
#include <Vorb/graphics/GLProgram.h>
#include "IRenderStage.h"

class ColorFilterRenderStage : public IRenderStage {
public:
    ColorFilterRenderStage();
    ~ColorFilterRenderStage();

    void init(vg::FullQuadVBO* quad);

    /// Draws the render stage
    virtual void render() override;

    void setColor(const f32v4 color) { m_color = color; }
private:
    f32v4 m_color;
    vg::FullQuadVBO* m_quad = nullptr;
};

#endif // ColorFilterRenderStage_h__
