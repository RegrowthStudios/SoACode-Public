#pragma once

#ifndef SpaceRenderStage_h__
#define SpaceRenderStage_h__

#include "IRenderStage.h"
#include "GLProgram.h"

class SkyboxRenderer;
class Camera;

class SpaceRenderStage : public vg::IRenderStage
{
public:
    SpaceRenderStage(vg::GLProgram* program,
                     Camera* camera);
    ~SpaceRenderStage();

    virtual void setState(vg::FrameBuffer* frameBuffer = nullptr) override;

    virtual void draw() override;

    virtual bool isVisible() override;
private:
    void drawSpace(glm::mat4 &VP);
    // Temporary until we have an actual sun
    void drawSun(float theta, glm::mat4 &MVP);

    SkyboxRenderer* _skyboxRenderer; ///< Renders the skybox

    vg::GLProgram* _glProgram;

    Camera* _camera;
};

#endif // SpaceRenderStage_h__
