#pragma once

#ifndef PlanetRenderStage_h__
#define PlanetRenderStage_h__

#include "IRenderStage.h"

class Camera;

class PlanetRenderStage : public vg::IRenderStage
{
public:
    PlanetRenderStage(Camera* camera);
    ~PlanetRenderStage();

    virtual void setState(vg::FrameBuffer* frameBuffer = nullptr) override;

    virtual void draw() override;

    virtual bool isVisible() override;

    Camera* _camera;
};
#endif // PlanetRenderStage_h__

