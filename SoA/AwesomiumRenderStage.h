#pragma once

#ifndef AwesomiumRenderStage_h__
#define AwesomiumRenderStage_h__

#include "IRenderStage.h"
#include "GLProgram.h"

class IAwesomiumInterface;

class AwesomiumRenderStage : public vg::IRenderStage {
public:
    AwesomiumRenderStage(IAwesomiumInterface* awesomiumInterface, vg::GLProgram* glProgram);
    ~AwesomiumRenderStage();

    virtual void setState(vg::FrameBuffer* frameBuffer = nullptr) override;

    virtual void draw() override;

    virtual bool isVisible() override;
private:
    IAwesomiumInterface* _awesomiumInterface;
    vg::GLProgram* _glProgram;
};

#endif // AwesomiumRenderStage_h__