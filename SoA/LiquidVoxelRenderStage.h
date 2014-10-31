#pragma once

#ifndef LiquidVoxelRenderStage_h__
#define LiquidVoxelRenderStage_h__

#include "IRenderStage.h"

class LiquidVoxelRenderStage : public vg::IRenderStage
{
public:
    LiquidVoxelRenderStage();
    ~LiquidVoxelRenderStage();

    virtual void setState(vg::FrameBuffer* frameBuffer = nullptr) override;

    virtual void draw() override;

    virtual bool isVisible() override;

};

#endif // LiquidVoxelRenderStage_h__