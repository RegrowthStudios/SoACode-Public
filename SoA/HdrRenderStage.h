#pragma once

#ifndef HdrRenderStage_h__
#define HdrRenderStage_h__

#include "IRenderStage.h"
#include "GLProgram.h"

class HdrRenderStage : public vg::IRenderStage {
public:
    HdrRenderStage(vg::GLProgram* glProgram, const ui32v2& destViewport);
    ~HdrRenderStage();

    virtual void setState(vg::FrameBuffer* frameBuffer = nullptr) override;

    virtual void draw() override;

    virtual bool isVisible() override;
private:
    vg::GLProgram* _glProgram;
    ui32v2 _destViewport;
};

#endif // HdrRenderStage_h__