#pragma once

#ifndef MainMenuRenderPipeline_h__
#define MainMenuRenderPipeline_h__

#include "IRenderPipeline.h"
#include "GLProgramManager.h"
#include "FrameBuffer.h"

class SpaceRenderStage;
class PlanetRenderStage;
class AwesomiumRenderStage;
class HdrRenderStage;
class Camera;
class IAwesomiumInterface;

class MainMenuRenderPipeline : public vg::IRenderPipeline 
{
public:
    MainMenuRenderPipeline();
    ~MainMenuRenderPipeline();

    void init(const ui32v2& viewport, Camera* camera, IAwesomiumInterface* awesomiumInterface, vg::GLProgramManager* glProgramManager);

    virtual void render() override;

    virtual void destroy() override;
private:
    SpaceRenderStage* _spaceRenderStage;
    PlanetRenderStage* _planetRenderStage;
    AwesomiumRenderStage* _awesomiumRenderStage;
    HdrRenderStage* _hdrRenderStage;

    vg::FrameBuffer* _hdrFrameBuffer;

    ui32v2 _viewport;
};

#endif // MainMenuRenderPipeline_h__
