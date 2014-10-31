#pragma once

#ifndef SonarRenderStage_h__
#define SonarRenderStage_h__

#include "IRenderStage.h"

class Camera;
class MeshManager;

class SonarRenderStage : public vg::IRenderStage
{
public:
    SonarRenderStage(Camera* camera, MeshManager* meshManager);
    ~SonarRenderStage();

    virtual void setState(vg::FrameBuffer* frameBuffer = nullptr) override;

    virtual void draw() override;

    virtual bool isVisible() override;
private:
    MeshManager* _meshManager;
};

#endif // SonarRenderStage_h__
