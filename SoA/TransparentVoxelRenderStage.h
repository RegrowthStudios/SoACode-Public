#pragma once

#ifndef TransparentVoxelRenderStage_h__
#define TransparentVoxelRenderStage_h__

#include "IRenderStage.h"

class GameRenderParams;
class Camera;
class MeshManager;

class TransparentVoxelRenderStage : public vg::IRenderStage {
public:
    TransparentVoxelRenderStage(Camera* camera, GameRenderParams* gameRenderParams, MeshManager* meshManager);
    ~TransparentVoxelRenderStage();

    virtual void setState(vg::FrameBuffer* frameBuffer = nullptr) override;

    virtual void draw() override;

    virtual bool isVisible() override;

private:
    GameRenderParams* _gameRenderParams;
    MeshManager* _meshManager;
};

#endif // TransparentVoxelRenderStage_h__

